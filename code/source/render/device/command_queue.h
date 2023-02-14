#pragma once

#include "def.h"

/* Project namespace */
namespace gdr
{

  // None value for this module
  const UINT64 NoneValue = (UINT64)-1;

  // command list representation class
  class command_list
  {
  public:
    // Init function
    bool Init(ID3D12Device* pDevice, const D3D12_COMMAND_LIST_TYPE& type);
    // Term function
    void Term();

    // Open command list with some fence function
    HRESULT Open(UINT64 fenceValue);
    // Close command list
    HRESULT Close();
    // Submit command list to command queue
    HRESULT Submit(ID3D12CommandQueue* pQueue);

    // Sync and return fence
    HRESULT Wait(UINT64& finishedFenceValue);

    // Return current command list
    inline ID3D12GraphicsCommandList* GetGraphicsCommandList() const { return CommandList; }
    // Return Fence
    inline ID3D12Fence* GetFence() const { return Fence; }
    // Return Event
    inline HANDLE GetEvent() const { return hEvent; }
    // Return Submitted fence 
    inline UINT64 GetSubmittedFenceValue() const { return SubmittedFenceValue; }
    // Return Pending fence
    inline UINT64 GetPendingFenceValue() const { return PendingFenceValue; }

  private:
    UINT64 CurrentFenceValue = NoneValue;   // current fence
    UINT64 PendingFenceValue = NoneValue;   // pending fence
    UINT64 SubmittedFenceValue = NoneValue; // submitted fence

    ID3D12CommandAllocator* CommandAllocator = nullptr; // allocator for commands
    ID3D12GraphicsCommandList* CommandList = nullptr;   // command List we are using
    ID3D12CommandList* CommandListForSubmit = nullptr;  // command list which are prepared to submit

    ID3D12Fence* Fence = nullptr;        // Fence for syncronization
    HANDLE hEvent = INVALID_HANDLE_VALUE; // Event 
  };

  /* command queue representation class */
  class command_queue
  {
  public:
    // virtual destructor
    virtual ~command_queue() {}

    // Init func
    bool Init(ID3D12Device* pDevice, const D3D12_COMMAND_LIST_TYPE& type, int cmdListCount);
    // Terminate func
    void Term();

    // Open commamd list
    HRESULT OpenCommandList(ID3D12GraphicsCommandList** ppList, UINT64& finishedFenceValue);
    // Close command list
    HRESULT CloseCommandList();
    // Submit command list
    HRESULT SubmitCommandList(UINT64* pSubmitFenceValue = nullptr);
    // Close and submit command list
    HRESULT CloseAndSubmitCommandList(UINT64* pSubmitFenceValue = nullptr);

    // Sync command list inside and return fence
    void WaitIdle(UINT64& finishedFenceValue);

    // get ID3D12CommandQueue
    inline ID3D12CommandQueue* GetQueue() const { return Queue; }
    // get command list
    inline command_list* GetCurrentCommandList() const { return CmdLists[CurCmdList]; }
    // get command lists amount
    inline size_t GetCommandListCount() const { return CmdLists.size(); }

  protected:
    virtual HRESULT PreSignal() { return S_OK; }

  private:
    ID3D12CommandQueue* Queue = nullptr; // our Queue of commands

    static UINT64 CurrentFenceValue; // value of current fence

    int CurCmdList = 0; // current command list number
    std::vector<command_list*> CmdLists; // all command lists
  };

  // Command queue for presenting data on screen 
  class present_command_queue : public command_queue
  {
  public:
    // virtual destructor
    virtual ~present_command_queue() {}

    // set swapchain
    inline void SetSwapchain(IDXGISwapChain3* pSwapchain) { Swapchain = pSwapchain; }

    // Set vertical synchronization
    inline void SetVSync(bool vsync) { VSync = vsync; }

  protected:
    virtual HRESULT PreSignal() override;

  private:
    IDXGISwapChain3* Swapchain = nullptr; // Swapchain for presenting
    bool VSync = true; // vsync flag
  };

  // Command queue for uploading
  class upload_command_queue : public command_queue
  {
  public:
    // virtual destructor
    virtual ~upload_command_queue() {}
  };



}