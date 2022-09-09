#include "p_header.h"

bool gdr::command_list::Init(ID3D12Device* pDevice, const D3D12_COMMAND_LIST_TYPE& type)
{
  HRESULT hr = S_OK;
  D3D_CHECK(pDevice->CreateCommandAllocator(type, __uuidof(ID3D12CommandAllocator), (void**)&CommandAllocator));
  D3D_CHECK(pDevice->CreateCommandList(0, type, CommandAllocator, nullptr, __uuidof(ID3D12CommandList), (void**)&CommandListForSubmit));
  D3D_CHECK(CommandListForSubmit->QueryInterface(__uuidof(ID3D12GraphicsCommandList), (void**)&CommandList));
  D3D_CHECK(CommandList->Close());
  D3D_CHECK(CommandAllocator->Reset());
  D3D_CHECK(pDevice->CreateFence(NoneValue, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&Fence));
  if (SUCCEEDED(hr))
  {
    hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    hr = hEvent != INVALID_HANDLE_VALUE ? S_OK : E_FAIL;
  }

  if (!SUCCEEDED(hr))
  {
    Term();
  }

  return SUCCEEDED(hr);
}

void gdr::command_list::Term()
{
  if (hEvent != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hEvent);
    hEvent = INVALID_HANDLE_VALUE;
  }

  D3D_RELEASE(Fence);

  D3D_RELEASE(CommandListForSubmit);
  D3D_RELEASE(CommandList);
  D3D_RELEASE(CommandAllocator);
}

HRESULT gdr::command_list::Open(UINT64 fenceValue)
{
  assert(PendingFenceValue == NoneValue && SubmittedFenceValue == NoneValue && CurrentFenceValue == NoneValue);

  HRESULT hr = S_OK;
  D3D_CHECK(CommandAllocator->Reset());
  D3D_CHECK(CommandList->Reset(CommandAllocator, nullptr));

  CurrentFenceValue = fenceValue;

  return hr;
}

HRESULT gdr::command_list::Close()
{
  assert(PendingFenceValue == NoneValue && SubmittedFenceValue == NoneValue && CurrentFenceValue != NoneValue);

  HRESULT hr = S_OK;
  D3D_CHECK(CommandList->Close());

  PendingFenceValue = CurrentFenceValue;
  CurrentFenceValue = NoneValue;

  return hr;
}

HRESULT gdr::command_list::Submit(ID3D12CommandQueue* pQueue)
{
  assert(PendingFenceValue != NoneValue && SubmittedFenceValue == NoneValue && CurrentFenceValue == NoneValue);

  pQueue->ExecuteCommandLists(1, &CommandListForSubmit);

  SubmittedFenceValue = PendingFenceValue;
  PendingFenceValue = NoneValue;

  return S_OK;
}

HRESULT gdr::command_list::Wait(UINT64& finishedFenceValue)
{
  assert(PendingFenceValue == NoneValue && CurrentFenceValue == NoneValue);

  finishedFenceValue = NoneValue;

  HRESULT hr = S_OK;
  if (SubmittedFenceValue != NoneValue)
  {
    UINT64 completed = Fence->GetCompletedValue();
    if (completed == NoneValue || completed < SubmittedFenceValue)
    {
      D3D_CHECK(Fence->SetEventOnCompletion(SubmittedFenceValue, hEvent));
      if (SUCCEEDED(hr))
      {
        WaitForSingleObject(hEvent, INFINITE);
        completed = Fence->GetCompletedValue();
        assert(completed != NoneValue && completed >= SubmittedFenceValue);
      }
    }
    finishedFenceValue = SubmittedFenceValue;
    SubmittedFenceValue = NoneValue;
  }
  return hr;
}

bool gdr::command_queue::Init(ID3D12Device* pDevice, const D3D12_COMMAND_LIST_TYPE& type, int cmdListCount)
{
  bool res = true;
  for (int i = 0; i < cmdListCount && res; i++)
  {
    command_list* pList = new command_list();
    res = pList->Init(pDevice, type);
    if (res)
    {
      CmdLists.push_back(pList);
    }
    else
    {
      delete pList;
      pList = nullptr;
    }
  }

  if (res)
  {
    HRESULT hr = S_OK;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    D3D_CHECK(pDevice->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), (void**)&Queue));

    res = SUCCEEDED(hr);
  }

  if (res)
  {
    CurrentFenceValue = 1;
  }

  // Warm-up fences
  if (res)
  {
    HRESULT hr = S_OK;
    for (int i = 0; i < cmdListCount && SUCCEEDED(hr); i++)
    {
      ID3D12Fence* pFence = CmdLists[i]->GetFence();
      D3D_CHECK(Queue->Signal(pFence, CurrentFenceValue + i));
    }

    for (int i = 0; i < cmdListCount && SUCCEEDED(hr); i++)
    {
      ID3D12Fence* pFence = CmdLists[i]->GetFence();
      HANDLE hEvent = CmdLists[i]->GetEvent();

      if (pFence->GetCompletedValue() == NoneValue || pFence->GetCompletedValue() < CurrentFenceValue)
      {
        D3D_CHECK(pFence->SetEventOnCompletion(CurrentFenceValue, hEvent));
        hr = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0 ? S_OK : E_FAIL;
        assert(SUCCEEDED(hr));
      }

      ++CurrentFenceValue;
    }
  }

  if (!res)
  {
    Term();
  }

  return res;
}

void gdr::command_queue::Term()
{
  UINT64 finishedFenceValue; // Will not be actually used here
  WaitIdle(finishedFenceValue);

  D3D_RELEASE(Queue);

  for (auto pList : CmdLists)
  {
    pList->Term();
    delete pList;
  }
  CmdLists.clear();

  CurrentFenceValue = NoneValue;
  CurCmdList = -1;
}

HRESULT gdr::command_queue::OpenCommandList(ID3D12GraphicsCommandList** ppList, UINT64& finishedFenceValue)
{
  CurCmdList = (CurCmdList + 1) % CmdLists.size();

  command_list* pCmdList = CmdLists[CurCmdList];

  finishedFenceValue = NoneValue;

  HRESULT hr = S_OK;
  D3D_CHECK(pCmdList->Wait(finishedFenceValue));
  D3D_CHECK(pCmdList->Open(CurrentFenceValue));

  if (SUCCEEDED(hr))
  {
    *ppList = pCmdList->GetGraphicsCommandList();
    ++CurrentFenceValue;
  }

  return hr;
}

HRESULT gdr::command_queue::CloseAndSubmitCommandList(UINT64* pSubmitFenceValue)
{
  if (pSubmitFenceValue != nullptr)
  {
    *pSubmitFenceValue = NoneValue;
  }

  command_list* pCmdList = CmdLists[CurCmdList];

  HRESULT hr = CloseCommandList();
  if (SUCCEEDED(hr))
  {
    hr = SubmitCommandList(pSubmitFenceValue);
  }

  return hr;
}

HRESULT gdr::command_queue::CloseCommandList()
{
  command_list* pCmdList = CmdLists[CurCmdList];

  HRESULT hr = S_OK;
  D3D_CHECK(pCmdList->Close());

  return hr;
}

HRESULT gdr::command_queue::SubmitCommandList(UINT64* pSubmitFenceValue)
{
  if (pSubmitFenceValue != nullptr)
  {
    *pSubmitFenceValue = NoneValue;
  }

  if (CurCmdList == -1)
  {
    return S_OK;
  }

  command_list* pCmdList = CmdLists[CurCmdList];
  if (pCmdList->GetPendingFenceValue() == NoneValue)
  {
    return S_OK;
  }

  HRESULT hr = S_OK;
  D3D_CHECK(pCmdList->Submit(Queue));
  D3D_CHECK(PreSignal());
  D3D_CHECK(Queue->Signal(pCmdList->GetFence(), pCmdList->GetSubmittedFenceValue()));

  if (SUCCEEDED(hr))
  {
    if (pSubmitFenceValue != nullptr)
    {
      *pSubmitFenceValue = pCmdList->GetSubmittedFenceValue();
    }
  }

  return hr;
}

void gdr::command_queue::WaitIdle(UINT64& finishedFenceValue)
{
  finishedFenceValue = NoneValue;
  for (auto pCmdList : CmdLists)
  {
    UINT64 cmdListFinishedFenceValue = NoneValue;

    if (pCmdList->GetSubmittedFenceValue() != NoneValue)
    {
      pCmdList->Wait(cmdListFinishedFenceValue);
      if (cmdListFinishedFenceValue != NoneValue)
      {
        finishedFenceValue = cmdListFinishedFenceValue;
      }
    }
  }
}

HRESULT gdr::present_command_queue::PreSignal()
{
  return Swapchain->Present(VSync ? 1 : 0, 0);
}
