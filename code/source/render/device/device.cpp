#include "p_header.h"

#define RELEASE(a)\
if ((a) != nullptr)\
{\
    (a)->Term();\
    delete (a);\
    (a) = nullptr;\
}

// Default constructor
gdr::device::device() : 
  IsInited(false),
  D3DGPUMemAllocator(nullptr),
  DxgiFactory(nullptr),
  DxgiAdapter(nullptr),
  D3DDevice(nullptr),
  D3DDescriptorHeap(nullptr),
  IsDebugShaders(false),
  IsDebugDevice(false),
  RtvDescSize(0),
  DsvDescSize(0),
  SrvDescSize(0)
{
}

/* Initialize function
 * ARGUMENTS:
 *      - Engine to init
 *          (engine *) Eng
 * RETURNS: true if success, false otherwise
 */
bool gdr::device::Init(const device_create_params& params)
{
  IsInited = true;
  IsInited = IsInited && InitD3D12Device(params.DebugLayer);
  IsInited = IsInited && InitCommandQueue(params.CmdListCount, params.UploadListCount);
  IsInited = IsInited && InitSwapchain(params.CmdListCount, params.hWnd);
  IsInited = IsInited && InitGPUMemoryAllocator();
  IsInited = IsInited && InitDescriptorHeap(params.StaticDescCount + params.DynamicDescCount);
  IsInited = IsInited && InitUploadEngine((UINT64)1 * params.UploadHeapSizeMb * 1024 * 1024, (UINT64)1 * params.DynamicHeapSizeMb * 1024 * 1024, params.DynamicDescCount);
  IsInited = IsInited && InitRenderTargetViewHeap(params.RenderTargetViewCount);
  IsInited = IsInited && InitReadbackEngine((UINT64)1 * params.ReadbackHeapSizeMb * 1024 * 1024);
  IsInited = IsInited && InitQueries(params.QueryCount);

  return IsInited;
}

/* Terminate function
 * ARGUMENTS: None
 * RETURNS: None
 */
void gdr::device::Term(void)
{
  if (IsInited)
  {
    TermQueries();
    TermReadbackEngine();
    TermRenderTargetViewHeap();
    TermUploadEngine();
    TermDescriptorHeap();
    TermGPUMemoryAllocator();
    TermSwapchain();
    TermCommandQueue();
    TermD3D12Device();
  }
}

// Default destructor
gdr::device::~device()
{
  Term();
}

// Init D3D12 Device
// ARGUMENTS: (bool) is enable debug 
bool gdr::device::InitD3D12Device(bool DebugDevice)
{
  DxgiFactory = nullptr;

  IsDebugDevice = DebugDevice;

  bool strictDebug = false;

  HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&DxgiFactory);
  assert(SUCCEEDED(hr));

  if (IsDebugDevice)
  {
    ID3D12Debug1* pDebug = nullptr;
    HRESULT hr = D3D12GetDebugInterface(__uuidof(ID3D12Debug1), (void**)&pDebug);
    assert(SUCCEEDED(hr));

    pDebug->EnableDebugLayer();
    {
      ID3D12DeviceRemovedExtendedDataSettings1* pDredSettings = nullptr;
      hr = D3D12GetDebugInterface(__uuidof(ID3D12DeviceRemovedExtendedDataSettings1), (void**)(&pDredSettings));

      // Turn on AutoBreadcrumbs and Page Fault reporting
      pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
      pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
    }

    if (strictDebug)
    {
      pDebug->SetEnableGPUBasedValidation(TRUE);
    }

    pDebug->Release();
  }

  // Find suitable GPU adapter
  UINT adapterIdx = 0;
  IDXGIAdapter* pAdapter = nullptr;
  UINT maxAdapterIdx = 0;
  UINT maxMemory = 0;
  while (DxgiFactory->EnumAdapters(adapterIdx, &pAdapter) == S_OK)
  {
    bool skip = false;

    DXGI_ADAPTER_DESC desc;
    pAdapter->GetDesc(&desc);

    skip = wcscmp(desc.Description, L"Microsoft Basic Render Driver") == 0;
    
    if (!skip)
    {
      skip = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&D3DDevice) != S_OK;
      
      // Fall back to 11_1 feature level, if 12_0 is not supported
      if (skip)
      {
        OutputDebugString(_T("Feature level 12_0 is not supported on "));
        OutputDebugStringW(desc.Description);
        OutputDebugString(_T(", fall back to 11_1\n"));
        skip = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&D3DDevice) != S_OK;
      }
    }

    if (!skip)
    {
      if (desc.DedicatedVideoMemory > maxMemory)
      {
        maxAdapterIdx = adapterIdx;
        maxMemory = desc.DedicatedVideoMemory;
      }
      D3DDevice->Release();
    }

    pAdapter->Release();

    adapterIdx++;
  }

  DxgiFactory->EnumAdapters(maxAdapterIdx, &pAdapter);
  bool skip = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&D3DDevice) != S_OK;

  // Fall back to 11_1 feature level, if 12_0 is not supported
  if (skip)
  {
    OutputDebugString(_T("Feature level 12_0 is not supported, fall back to 11_1\n"));
    skip = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&D3DDevice) != S_OK;
  }

  DxgiAdapter = pAdapter;

  if (DxgiAdapter != nullptr)
  {
    IDXGIOutput* pOutput = nullptr;
    int outputIndex = 0;
    while (DxgiAdapter->EnumOutputs(outputIndex, &pOutput) == S_OK)
    {
      DXGI_OUTPUT_DESC desc;
      pOutput->GetDesc(&desc);

      pOutput->Release();

      ++outputIndex;
    }
  }

  if (D3DDevice != nullptr)
  {
    SrvDescSize = D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    DsvDescSize = D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    RtvDescSize = D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }

  if (D3DDevice != nullptr && DebugDevice)
  {
    ID3D12InfoQueue* pInfoQueue = nullptr;

    HRESULT hr = S_OK;

    D3D_CHECK(D3DDevice->QueryInterface(__uuidof(ID3D12InfoQueue), (void**)&pInfoQueue));
    if (strictDebug)
    {
      D3D_CHECK(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE));
    }
    D3D_CHECK(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE));
    D3D_CHECK(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE));
    D3D_RELEASE(pInfoQueue);
  }

  return D3DDevice != nullptr;
}

// Terminate D3D12 Device
void gdr::device::TermD3D12Device(void)
{
  ULONG refs = 0;
  if (D3DDevice != nullptr)
  {
    refs = D3DDevice->Release();
    D3DDevice = nullptr;
  }
  
  if (IsDebugDevice && refs)
  {
    IDXGIDebug1* pDxgiDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pDxgiDebug)))
    {
      pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL));
      D3D_RELEASE(pDxgiDebug);
    }
  }
  
  D3D_RELEASE(DxgiAdapter);
  D3D_RELEASE(DxgiFactory);
}

// Init Command buffer
// ARGUEMTNS: Amount of command queues and amount of Upload command queues
bool gdr::device::InitCommandQueue(int Count, int UploadCount)
{
  PresentQueue = new present_command_queue();
  bool res = PresentQueue->Init(D3DDevice, D3D12_COMMAND_LIST_TYPE_DIRECT, Count);
  if (res)
  {
    UploadQueue = new upload_command_queue();
    res = UploadQueue->Init(D3DDevice, D3D12_COMMAND_LIST_TYPE_COPY, UploadCount);
  }
  if (res)
  {
    UploadStateTransitionQueue = new command_queue();
    res = UploadStateTransitionQueue->Init(D3DDevice, D3D12_COMMAND_LIST_TYPE_DIRECT, Count);
  }
  return res;
}

// Terminate Command buffer
void gdr::device::TermCommandQueue(void)
{
  RELEASE(UploadStateTransitionQueue);
  RELEASE(UploadQueue);
  RELEASE(PresentQueue);
}

// Init Swapchain
// ARGUEMTNS: Amount of swapchains and hWnd
bool gdr::device::InitSwapchain(int Count, HWND hWnd)
{
  DXGI_MODE_DESC bufferDesc;
  bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  bufferDesc.Height = 0;
  bufferDesc.Width = 0;
  bufferDesc.RefreshRate.Numerator = 0;
  bufferDesc.RefreshRate.Denominator = 1;
  bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
  bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

  DXGI_SWAP_CHAIN_DESC desc;
  desc.BufferCount = Count;
  desc.BufferDesc = bufferDesc;
  desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.OutputWindow = hWnd;
  desc.Windowed = TRUE;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  desc.Flags = 0;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;

  IDXGISwapChain* pSwapchain = nullptr;

  HRESULT hr = S_OK;
  D3D_CHECK(DxgiFactory->CreateSwapChain(PresentQueue->GetQueue(), &desc, &pSwapchain));
  D3D_CHECK(pSwapchain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&Swapchain));
  if (hr != S_OK)
  {
    OutputDebugString(_T("DXGI Swapchain version 1.4 is not supported\n"));
  }
  D3D_RELEASE(pSwapchain);

  if (SUCCEEDED(hr))
  {
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = Count;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    D3D_CHECK(D3DDevice->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap), (void**)&BackBufferViews));
  }

  D3D_CHECK(CreateBackBuffers(Count));

  return SUCCEEDED(hr);
}

/* Init Back buffers
 * ARGUMENTS: Amount of BackBuffers
 * RETURNS: (HRESULT) return code of creation
 */
HRESULT gdr::device::CreateBackBuffers(UINT Count)
{
  HRESULT hr = S_OK;

  for (UINT i = 0; i < Count && SUCCEEDED(hr); i++)
  {
    ID3D12Resource* pBackBuffer = nullptr;
    D3D_CHECK(Swapchain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&pBackBuffer));
    BackBuffers.push_back(pBackBuffer);
  }

  if (SUCCEEDED(hr))
  {
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(BackBufferViews->GetCPUDescriptorHandleForHeapStart());

    D3D12_RENDER_TARGET_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;
    desc.Texture2D.PlaneSlice = 0;

    for (UINT i = 0; i < Count; i++)
    {
      D3DDevice->CreateRenderTargetView(BackBuffers[i], &desc, rtvHandle);

      rtvHandle.ptr += RtvDescSize;
    }
  }

  if (SUCCEEDED(hr))
  {
    BackBufferCount = Count;

    // We actually should point at previous to grab current at new frame
    CurrentBackBufferIdx = (Swapchain->GetCurrentBackBufferIndex() + BackBufferCount - 1) % BackBufferCount;
  }

  return hr;
}

// Terminate back buffers
void gdr::device::TermBackBuffers()
{
  for (auto pBackBuffer : BackBuffers)
  {
    D3D_RELEASE(pBackBuffer);
  }
  BackBuffers.clear();

  BackBufferCount = 0;
  CurrentBackBufferIdx = 0;
}

// Terminate Swapchain
void gdr::device::TermSwapchain(void)
{
  D3D_RELEASE(BackBufferViews);

  TermBackBuffers();

  D3D_RELEASE(Swapchain);
}

// Init GPU Memory Allocator
bool gdr::device::InitGPUMemoryAllocator(void)
{
  return true;
}

// Terminate GPU Memory Allocator
void gdr::device::TermGPUMemoryAllocator(void)
{

}

// Init Descriptor Heap
// ARGUMENTS: Total Amount of descriptors
bool gdr::device::InitDescriptorHeap(UINT DescCount)
{
  return true;
}

// Terminate Descriptor Heap
void gdr::device::TermDescriptorHeap(void)
{

}

/* Init Upload Engine
 * ARGUMENTS:
 *        - size of upload heap in bytes
 *             UINT64 UploadHeapSize
 *        - size of Dynamic heap in bytes
 *             UINT64 DynamicHeapSize
 *        - amount of dynamic descriptors
 *             UINT DynamicDescCount
 */
bool gdr::device::InitUploadEngine(UINT64 UploadHeapSize, UINT64 DynamicHeapSize, UINT DynamicDescCount)
{
  return true;
}

// Terminate Upload Engine
void gdr::device::TermUploadEngine(void)
{

}

/* Init RenderTargetViewHeap
 * ARGUMENTS: Amount of render targets
 */
bool gdr::device::InitRenderTargetViewHeap(UINT Count)
{
  return true;
}

// Terminate RenderTargetViewHeap
void gdr::device::TermRenderTargetViewHeap(void)
{

}

/* Init Readback Engine
 * ARGUMENTS: Size of Readback Heap in bytes
 */
bool gdr::device::InitReadbackEngine(UINT64 ReadbackHeapSize)
{
  return true;
}

// Terminate Readback Engine
void gdr::device::TermReadbackEngine(void)
{

}

/* Init Queries
 * ARGUMENTS: Amount of Queries
 */
bool gdr::device::InitQueries(UINT Count)
{
  return true;
}

// Terminate Queries
void gdr::device::TermQueries(void)
{

}