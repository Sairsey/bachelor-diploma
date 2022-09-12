#include "p_header.h"

// Default constructor
gdr::render::render() : IsInited(false)
{
}

// Create Depth-stencil buffer
bool gdr::render::CreateDepthStencil(void)
{
  D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
  depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
  depthOptimizedClearValue.DepthStencil.Stencil = 0;

  bool res = Device.CreateGPUResource(
    CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, Rect.right - Rect.left, Rect.bottom - Rect.top, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &depthOptimizedClearValue,
    DepthBuffer
  );
  if (res)
  {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvView = {};
    dsvView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvView.Flags = D3D12_DSV_FLAG_NONE;

    Device.GetDXDevice()->CreateDepthStencilView(DepthBuffer.Resource, &dsvView, DSVHeap->GetCPUDescriptorHandleForHeapStart());
  }

  return res;
}

/* Initialize function
 * ARGUMENTS:
 *      - Engine to init
 *          (engine *) Eng
 * RETURNS: true if success, false otherwise
 */
bool gdr::render::Init(engine* Eng)
{
  Engine = Eng;
  bool localIsInited = true;

  gdr::device_create_params params;
  params.hWnd = Eng->hWnd;

  // init device
  localIsInited = Device.Init(params);

  // init sizes
  if (localIsInited) 
  {
    auto size = Eng->GetSize();

    Viewport.TopLeftX = 0.0f;
    Viewport.TopLeftY = 0.0f;
    Viewport.Height = (LONG)size.second;
    Viewport.Width = (LONG)size.first;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;

    Rect.left = 0;
    Rect.top = 0;
    Rect.bottom = (LONG)size.second;
    Rect.right = (LONG)size.first;
  }

  // init depth-stencil
  if (localIsInited)
  {
    D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
    dsvDesc.NumDescriptors = 1;
    dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

    HRESULT hr = S_OK;
    D3D_CHECK(Device.GetDXDevice()->CreateDescriptorHeap(&dsvDesc, __uuidof(ID3D12DescriptorHeap), (void**)&DSVHeap));
    localIsInited = SUCCEEDED(hr);
    localIsInited = localIsInited && CreateDepthStencil();
  }

  // init passes
  if (localIsInited)
  {
    Passes.push_back(new albedo_pass());

    for (auto& pass : Passes)
    {
      pass->SetRender(this);
      pass->Initialize();
    }
  }

  //init subsystems
  if (localIsInited)
  {
    ID3D12GraphicsCommandList *commandList;
    Device.BeginUploadCommandList(&commandList);
    Geometry = new gdr::geometry_support(this);

    Device.CloseUploadCommandList();
  }

  IsInited = localIsInited;

  return IsInited;
}

/* Resize frame function
 * ARGUMENTS:
 *      - new frame width
 *          (UINT) w
 *      - new frame height
 *          (UINT) h
 * RETURNS: None
 */
void gdr::render::Resize(UINT w, UINT h)
{
  Device.ResizeSwapchain(w, h);
  Viewport.TopLeftX = 0.0f;
  Viewport.TopLeftY = 0.0f;
  Viewport.Height = (FLOAT)h;
  Viewport.Width = (FLOAT)w;
  Viewport.MinDepth = 0.0f;
  Viewport.MaxDepth = 1.0f;

  Rect.left = 0;
  Rect.top = 0;
  Rect.bottom = (FLOAT)h;
  Rect.right = (FLOAT)w;

  if (DepthBuffer.Resource != nullptr)
  {
    D3D12_RESOURCE_DESC desc = DepthBuffer.Resource->GetDesc();
    if (desc.Width != w || desc.Height != h)
    {
      Device.ReleaseGPUResource(DepthBuffer);

      CreateDepthStencil();
    }
  }
}

/* Draw frame function */
void gdr::render::DrawFrame(void)
{
  if (!IsInited)
    return;

  ID3D12GraphicsCommandList* pCommandList = nullptr;
  ID3D12Resource* pBackBuffer = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
  if (Device.BeginRenderCommandList(&pCommandList, &pBackBuffer, &rtvHandle))
  {
    HRESULT hr = S_OK;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DSVHeap->GetCPUDescriptorHandleForHeapStart();
    pCommandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &dsvHandle);

    pCommandList->RSSetViewports(1, &Viewport);
    pCommandList->RSSetScissorRects(1, &Rect);

    if (Device.TransitResourceState(pCommandList, pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET))
    {
      FLOAT clearColor[4] = { 0.3f, 0.5f, 0.7f, 1.0f };
      pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 1, &Rect);

      assert(Passes.size() >= 1);
      Passes[0]->CallCompute(pCommandList);
      for (int i = 0; i < Passes.size() - 1; i++)
      {
        Passes[i + 1]->CallCompute(pCommandList);
        Passes[i]->SyncCompute(pCommandList);
        if (Params.IsIndirect)
          Passes[i]->CallIndirectDraw(pCommandList);
        else
          Passes[i]->CallDirectDraw(pCommandList);
      }
      Passes[Passes.size() - 1]->SyncCompute(pCommandList);
      if (Params.IsIndirect)
        Passes[Passes.size() - 1]->CallIndirectDraw(pCommandList);
      else
        Passes[Passes.size() - 1]->CallDirectDraw(pCommandList);

      Device.TransitResourceState(pCommandList, pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    }

    Device.CloseSubmitAndPresentRenderCommandList();
  }

}

/* Initialize function
 * ARGUMENTS:
 *      - Engine to init
 *          (engine *) Eng
 * RETURNS: true if success, false otherwise
 */
void gdr::render::Term(void)
{
  if (!IsInited)
    return;

  for (auto& pass : Passes)
  {
    delete pass;
  }
  delete Geometry;
  Device.ReleaseGPUResource(DepthBuffer);
  DSVHeap->Release();
  Passes.clear();
  Device.Term();
}

// Default destructor
gdr::render::~render()
{
  Term();
}