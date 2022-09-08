#include "p_header.h"

// Default constructor
gdr::device::device() : 
  IsInited(false),
  D3DGPUMemAllocator(nullptr),
  DxgiFactory(nullptr),
  DxgiAdapter(nullptr),
  DxgiDevice(nullptr),
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
      skip = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&DxgiDevice) != S_OK;
      
      // Fall back to 11_1 feature level, if 12_0 is not supported
      if (skip)
      {
        OutputDebugString(_T("Feature level 12_0 is not supported on "));
        OutputDebugStringW(desc.Description);
        OutputDebugString(_T(", fall back to 11_1\n"));
        skip = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&DxgiDevice) != S_OK;
      }
    }

    if (!skip)
    {
      if (desc.DedicatedVideoMemory > maxMemory)
      {
        maxAdapterIdx = adapterIdx;
        maxMemory = desc.DedicatedVideoMemory;
      }
      DxgiDevice->Release();
    }

    pAdapter->Release();

    adapterIdx++;
  }

  DxgiFactory->EnumAdapters(maxAdapterIdx, &pAdapter);
  bool skip = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&DxgiDevice) != S_OK;

  // Fall back to 11_1 feature level, if 12_0 is not supported
  if (skip)
  {
    OutputDebugString(_T("Feature level 12_0 is not supported, fall back to 11_1\n"));
    skip = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&DxgiDevice) != S_OK;
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

  if (DxgiDevice != nullptr)
  {
    SrvDescSize = DxgiDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    DsvDescSize = DxgiDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    RtvDescSize = DxgiDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }

  if (DxgiDevice != nullptr && DebugDevice)
  {
    ID3D12InfoQueue* pInfoQueue = nullptr;

    HRESULT hr = S_OK;

    D3D_CHECK(DxgiDevice->QueryInterface(__uuidof(ID3D12InfoQueue), (void**)&pInfoQueue));
    if (strictDebug)
    {
      D3D_CHECK(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE));
    }
    D3D_CHECK(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE));
    D3D_CHECK(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE));
    D3D_RELEASE(pInfoQueue);
  }

  return DxgiDevice != nullptr;
}

// Terminate D3D12 Device
void gdr::device::TermD3D12Device(void)
{
  ULONG refs = 0;
  if (DxgiDevice != nullptr)
  {
    refs = DxgiDevice->Release();
    DxgiDevice = nullptr;
  }
  /*
  if (IsDebugDevice && refs)
  {
    IDXGIDebug1* pDxgiDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pDxgiDebug)))
    {
      pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL));
      D3D_RELEASE(pDxgiDebug);
    }
  }
  */

  D3D_RELEASE(DxgiAdapter);
  D3D_RELEASE(DxgiFactory);
}

// Init Command buffer
// ARGUEMTNS: Amount of command queues and amount of Upload command queues
bool gdr::device::InitCommandQueue(int Count, int UploadCount)
{
  return true;
}

// Terminate Command buffer
void gdr::device::TermCommandQueue(void)
{

}

// Init Swapchain
// ARGUEMTNS: Amount of swapchains and hWnd
bool gdr::device::InitSwapchain(int Count, HWND hWnd)
{
  return true;
}

// Terminate Swapchain
void gdr::device::TermSwapchain(void)
{

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