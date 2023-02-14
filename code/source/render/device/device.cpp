#include "p_header.h"

#define RELEASE(a)\
if ((a) != nullptr)\
{\
    (a)->Term();\
    delete (a);\
    (a) = nullptr;\
}

//#define TOO_MANY_SYNCS

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
  UsedForCreationParams = params;

  IsInited = true;
  IsInited = IsInited && InitD3D12Device(params.DebugLayer);
  IsInited = IsInited && InitCommandQueue(params.CmdListCount, params.UploadListCount);
  IsInited = IsInited && InitSwapchain(params.CmdListCount, params.hWnd);
  IsInited = IsInited && InitGPUMemoryAllocator();
  IsInited = IsInited && InitDescriptorHeap(params.StaticDescCount + params.DynamicDescCount);
  IsInited = IsInited && InitUploadEngine((UINT64)1 * params.LoadtimeUploadHeapSizeMb * 1024 * 1024, (UINT64)1 * params.DynamicHeapSizeMb * 1024 * 1024, params.DynamicDescCount);
  IsInited = IsInited && InitRenderTargetViewHeap(params.RenderTargetViewCount);
  IsInited = IsInited && InitReadbackEngine((UINT64)1 * params.ReadbackHeapSizeMb * 1024 * 1024);
  IsInited = IsInited && InitQueries(params.QueryCount);

  if (IsInited)
  {
    PresentQueue->SetSwapchain(Swapchain);

    IsDebugShaders = params.DebugShaders;

    StaticDescCount = params.StaticDescCount;
  }
  else
  {
    Term();
  }

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
  GDR_ASSERT(SUCCEEDED(hr));

  if (IsDebugDevice)
  {
    ID3D12Debug1* pDebug = nullptr;
    HRESULT hr = D3D12GetDebugInterface(__uuidof(ID3D12Debug1), (void**)&pDebug);
    GDR_ASSERT(SUCCEEDED(hr));

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
  size_t maxMemory = 0;
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
  DXGI_ADAPTER_DESC desc;
  pAdapter->GetDesc(&desc);
  bool skip = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&D3DDevice) != S_OK;
  OutputDebugString(desc.Description);

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
  D3D12MA::ALLOCATOR_DESC desc;
  desc.pAdapter = DxgiAdapter;
  desc.pDevice = D3DDevice;
  desc.pAllocationCallbacks = nullptr;
  desc.PreferredBlockSize = 0;
  desc.Flags = D3D12MA::ALLOCATOR_FLAG_SINGLETHREADED;

  HRESULT hr = S_OK;
  D3D_CHECK(D3D12MA::CreateAllocator(&desc, &D3DGPUMemAllocator));

  return SUCCEEDED(hr);
}

// Terminate GPU Memory Allocator
void gdr::device::TermGPUMemoryAllocator(void)
{
  D3D_RELEASE(D3DGPUMemAllocator);
}

// Init Descriptor Heap
// ARGUMENTS: Total Amount of descriptors
bool gdr::device::InitDescriptorHeap(UINT DescCount)
{
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  heapDesc.NodeMask = 0;
  heapDesc.NumDescriptors = DescCount;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

  HRESULT hr = S_OK;
  D3D_CHECK(D3DDevice->CreateDescriptorHeap(&heapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&D3DDescriptorHeap));

  return true;
}

// Terminate Descriptor Heap
void gdr::device::TermDescriptorHeap(void)
{
  D3D_RELEASE(D3DDescriptorHeap);
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
  bool res = true;
  if (res)
  {
    UploadBuffer = new heap_ring_buffer();
    res = UploadBuffer->Init(D3DDevice, D3D12_HEAP_TYPE_UPLOAD, UploadHeapSize);
  }
  if (res)
  {
    DynamicBuffer = new heap_ring_buffer();
    res = DynamicBuffer->Init(D3DDevice, D3D12_HEAP_TYPE_UPLOAD, DynamicHeapSize);
  }
  if (res)
  {
    DynamicDescBuffer = new descriptor_ring_buffer();
    res = DynamicDescBuffer->Init(D3DDevice, DynamicDescCount, D3DDescriptorHeap);
  }
  if (res)
  {
    this->DynamicDescCount = DynamicDescCount;
  }
  return res;
}

// Terminate Upload Engine
void gdr::device::TermUploadEngine(void)
{
  RELEASE(DynamicDescBuffer);
  RELEASE(DynamicBuffer);
  RELEASE(UploadBuffer);
}

/* Init RenderTargetViewHeap
 * ARGUMENTS: Amount of render targets
 */
bool gdr::device::InitRenderTargetViewHeap(UINT Count)
{
  HRESULT hr = S_OK;
  if (SUCCEEDED(hr))
  {
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = Count;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    D3D_CHECK(D3DDevice->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap), (void**)&RenderTargetViews));
  }
  if (SUCCEEDED(hr))
  {
    MaxRTViews = Count;
    CurrentRTView = 0;
  }

  return SUCCEEDED(hr);
}

// Terminate RenderTargetViewHeap
void gdr::device::TermRenderTargetViewHeap(void)
{
  MaxRTViews = 0;
  CurrentRTView = 0;

  D3D_RELEASE(RenderTargetViews);
}

/* Init Readback Engine
 * ARGUMENTS: Size of Readback Heap in bytes
 */
bool gdr::device::InitReadbackEngine(UINT64 ReadbackHeapSize)
{
  bool res = true;
  if (res)
  {
    ReadbackBuffer = new heap_ring_buffer();
    res = ReadbackBuffer->Init(D3DDevice, D3D12_HEAP_TYPE_READBACK, ReadbackHeapSize);
  }

  return res;
}

// Terminate Readback Engine
void gdr::device::TermReadbackEngine(void)
{
  RELEASE(ReadbackBuffer);
}

/* Init Queries
 * ARGUMENTS: Amount of Queries
 */
bool gdr::device::InitQueries(UINT Count)
{
  QueryBuffer = new query_ring_buffer();
  if (QueryBuffer->Init(D3DDevice, D3DGPUMemAllocator, D3D12_QUERY_HEAP_TYPE_TIMESTAMP, Count))
  {
    return true;
  }

  RELEASE(QueryBuffer);

  return false;
}

// Terminate Queries
void gdr::device::TermQueries(void)
{
  RELEASE(QueryBuffer);
}

/* Function for execution of all thing we need to do on frame start*/
bool gdr::device::BeginRenderCommandList(ID3D12GraphicsCommandList** ppCommandList, ID3D12Resource** ppBackBuffer, D3D12_CPU_DESCRIPTOR_HANDLE* pBackBufferDesc)
{
  CurrentBackBufferIdx = (CurrentBackBufferIdx + 1) % BackBufferCount;

  // Open command list
  UINT64 finishedFenceValue = NoneValue;

  HRESULT hr = PresentQueue->OpenCommandList(ppCommandList, finishedFenceValue);
  if (finishedFenceValue != NoneValue)
  {
    DynamicBuffer->FlashFenceValue(finishedFenceValue);
    DynamicDescBuffer->FlashFenceValue(finishedFenceValue);
    QueryBuffer->FlashFenceValue(finishedFenceValue);
  }
  if (SUCCEEDED(hr))
  {
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(BackBufferViews->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.ptr += RtvDescSize * CurrentBackBufferIdx;
    *pBackBufferDesc = rtvHandle;
    *ppBackBuffer = BackBuffers[CurrentBackBufferIdx];
  }

  return SUCCEEDED(hr);
}

/* Function for execution of all thing we need to do on frame End*/
bool gdr::device::CloseSubmitAndPresentRenderCommandList(bool vsync)
{
  // Close command list
  HRESULT hr = S_OK;

#ifdef TOO_MANY_SYNCS
  // Wait for upload CL-s
  UINT64 uploadFenceValue = NoneValue;
  UploadQueue->WaitIdle(uploadFenceValue);
  if (uploadFenceValue != NoneValue)
  {
    UploadBuffer->FlashFenceValue(uploadFenceValue);

    // sync present queue with our fence
    D3D_CHECK(PresentQueue->GetQueue()->Wait(UploadQueue->GetCurrentCommandList()->GetFence(), uploadFenceValue));
  }
#else
  // instead of waiting present CL, lets just Sync it 
  UINT64 uploadFenceValue = UploadQueue->GetCurrentCommandList()->GetSubmittedFenceValue();
  if (uploadFenceValue != NoneValue)
  {
    // sync update queue with our fence
    D3D_CHECK(PresentQueue->GetQueue()->Wait(UploadQueue->GetCurrentCommandList()->GetFence(), uploadFenceValue));
  }
#endif

  // Set Vsync
  PresentQueue->SetVSync(vsync);

  D3D_CHECK(QueryBuffer->Resolve(PresentQueue->GetCurrentCommandList()->GetGraphicsCommandList()));

  UINT64 presentFenceValue = NoneValue;
  D3D_CHECK(PresentQueue->CloseAndSubmitCommandList(&presentFenceValue));
  // Add fences for every buffer
  if (SUCCEEDED(hr))
  {
    DynamicBuffer->AddPendingFence(presentFenceValue);
    DynamicDescBuffer->AddPendingFence(presentFenceValue);
    QueryBuffer->AddPendingFence(presentFenceValue);
  }

  return SUCCEEDED(hr);
}

bool gdr::device::BeginUploadCommandList(ID3D12GraphicsCommandList** ppCommandList)
{
  HRESULT hr = S_OK;

  UINT64 finishedFenceValue = NoneValue;
  // Open and wait for previous commit completion
  D3D_CHECK(UploadQueue->OpenCommandList(ppCommandList, finishedFenceValue));
  if (finishedFenceValue != NoneValue)
  {
    UploadBuffer->FlashFenceValue(finishedFenceValue); // mark in Upload buffer that we already waited for finishedFenceValue.
  }

  CurrentUploadCmdList = *ppCommandList;

  return SUCCEEDED(hr);
}

bool gdr::device::AllocateRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, UINT count)
{
  GDR_ASSERT(CurrentRTView < MaxRTViews && RenderTargetViews != nullptr);

  if (CurrentRTView < MaxRTViews)
  {
    cpuHandle = RenderTargetViews->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += RtvDescSize * CurrentRTView;

    CurrentRTView += count;

    return true;
  }

  return false;
}

bool gdr::device::AllocateStaticDescriptors(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE& cpuStartHandle, D3D12_GPU_DESCRIPTOR_HANDLE& gpuStartHandle)
{
  GDR_ASSERT(CurrentStaticDescIndex + count <= StaticDescCount);

  cpuStartHandle = D3DDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
  cpuStartHandle.ptr += (CurrentStaticDescIndex + DynamicDescCount) * SrvDescSize;

  gpuStartHandle = D3DDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
  gpuStartHandle.ptr += (CurrentStaticDescIndex + DynamicDescCount) * SrvDescSize;

  CurrentStaticDescIndex += count;

  return true;
}

HRESULT gdr::device::UpdateBuffer(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pBuffer, const void* pData, size_t dataSize)
{
#ifdef _DEBUG
  D3D12_RESOURCE_DESC desc = pBuffer->GetDesc();
  GDR_ASSERT(desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER);
  GDR_ASSERT(desc.Width >= dataSize);
#endif

  GDR_ASSERT(CurrentUploadCmdList == pCommandList);

  UINT64 allocStartOffset = 0;
  UINT8* pAlloc = nullptr;
  auto allocRes = UploadBuffer->Alloc(dataSize, allocStartOffset, pAlloc, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
  GDR_ASSERT(allocRes == ring_buffer_result::Ok);
  if (allocRes == ring_buffer_result::Ok)
  {
    memcpy(pAlloc, pData, dataSize);

    pCommandList->CopyBufferRegion(pBuffer, 0, UploadBuffer->GetBuffer(), allocStartOffset, dataSize);

    return S_OK;
  }

  return E_FAIL;
}

HRESULT gdr::device::UpdateBufferOffset(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pBuffer, size_t bufferOffset, const void* pData, size_t dataSize)
{
#ifdef _DEBUG
  D3D12_RESOURCE_DESC desc = pBuffer->GetDesc();
  GDR_ASSERT(desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER);
  GDR_ASSERT(desc.Width >= dataSize + bufferOffset);
#endif

  GDR_ASSERT(CurrentUploadCmdList == pCommandList);

  UINT64 allocStartOffset = 0;
  UINT8* pAlloc = nullptr;
  auto allocRes = UploadBuffer->Alloc(dataSize, allocStartOffset, pAlloc, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
  GDR_ASSERT(allocRes == ring_buffer_result::Ok);
  if (allocRes == ring_buffer_result::Ok)
  {
    memcpy(pAlloc, pData, dataSize);

    pCommandList->CopyBufferRegion(pBuffer, bufferOffset, UploadBuffer->GetBuffer(), allocStartOffset, dataSize);

    return S_OK;
  }

  return E_FAIL;
}

// In case we need huge data to be copied
void gdr::device::WaitAllUploadLists(void)
{
  UINT64 finishedFenceValue = NoneValue;
  UploadQueue->WaitIdle(finishedFenceValue);
  if (finishedFenceValue != NoneValue)
  {
    UploadBuffer->FlashFenceValue(finishedFenceValue);
  }
}

HRESULT gdr::device::UpdateTexture(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pTexture, const void* pData, size_t dataSize)
{
  D3D12_RESOURCE_DESC desc = pTexture->GetDesc();
  GDR_ASSERT(desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);

  UINT64 total = 0;
  std::vector<UINT> numRows(desc.MipLevels * desc.DepthOrArraySize);
  std::vector<UINT64> rowSize(desc.MipLevels * desc.DepthOrArraySize);
  std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> placedFootprint(desc.MipLevels * desc.DepthOrArraySize);

  D3DDevice->GetCopyableFootprints(&desc, 0, desc.MipLevels * desc.DepthOrArraySize, 0, placedFootprint.data(), numRows.data(), rowSize.data(), &total);

  GDR_ASSERT(CurrentUploadCmdList == pCommandList);

  UINT64 width = desc.Width;
  UINT64 height = desc.Height;
  const UINT8* pSrcData = static_cast<const UINT8*>(pData);

  UINT64 allocStartOffset = 0;

  UINT8* pAlloc = nullptr;
  auto allocRes = UploadBuffer->Alloc(total, allocStartOffset, pAlloc, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
  GDR_ASSERT(allocRes == ring_buffer_result::Ok);
  if (allocRes != ring_buffer_result::Ok)
  {
    return E_FAIL;
  }

  UINT pixelSize = 0;
  switch (desc.Format)
  {
  case DXGI_FORMAT_R8G8B8A8_UNORM:
  case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
  case DXGI_FORMAT_R32_UINT:
    pixelSize = 4;
    break;
  
  case DXGI_FORMAT_A8_UNORM:
    pixelSize = 1;
    break;

  case DXGI_FORMAT_R32G32B32_FLOAT:
    pixelSize = 12;
    break;

  case DXGI_FORMAT_R32G32B32A32_FLOAT:
    pixelSize = 16;
    break;

  default:
    GDR_FAILED("Unknown format"); // Unknown format
    break;
  }

  UINT8* pDstData = static_cast<UINT8*>(pAlloc);

  for (int arrayIndex = 0; arrayIndex < desc.DepthOrArraySize; arrayIndex++)
  {
    width = desc.Width;
    height = desc.Height;
    for (int mipIndex = 0; mipIndex < desc.MipLevels; mipIndex++)
    {
      const int subResourceIndex = mipIndex + (arrayIndex * desc.MipLevels);

      // Copy from pData
      for (UINT j = 0; j < height; j++)
      {
        memcpy(pDstData, pSrcData, width * pixelSize);
        pDstData += placedFootprint[subResourceIndex].Footprint.RowPitch;
        pSrcData += width * pixelSize;
      }

      placedFootprint[subResourceIndex].Offset += allocStartOffset;

      const auto& dst = CD3DX12_TEXTURE_COPY_LOCATION(pTexture, subResourceIndex);
      const auto& src = CD3DX12_TEXTURE_COPY_LOCATION(UploadBuffer->GetBuffer(), placedFootprint[subResourceIndex]);
      pCommandList->CopyTextureRegion(
        &dst,
        0, 0, 0,
        &src,
        nullptr);

      width /= 2;
      height /= 2;
    }
  }

  return S_OK;
}

void gdr::device::CloseUploadCommandList()
{
  // Close command list
  HRESULT hr = S_OK;

#ifdef TOO_MANY_SYNCS
  // Wait for present CL-s
  UINT64 presentFenceValue = NoneValue;
  PresentQueue->WaitIdle(presentFenceValue);
  if (presentFenceValue != NoneValue)
  {
    DynamicBuffer->FlashFenceValue(presentFenceValue);
    DynamicDescBuffer->FlashFenceValue(presentFenceValue);
    QueryBuffer->FlashFenceValue(presentFenceValue);

    // sync update queue with our fence
    D3D_CHECK(UploadQueue->GetQueue()->Wait(PresentQueue->GetCurrentCommandList()->GetFence(), presentFenceValue));
  }
#else
  // instead of waiting present CL, lets just Sync it 
  UINT64 presentFenceValue = PresentQueue->GetCurrentCommandList()->GetSubmittedFenceValue();
  if (presentFenceValue != NoneValue)
  {
    // sync update queue with our fence
    D3D_CHECK(UploadQueue->GetQueue()->Wait(PresentQueue->GetCurrentCommandList()->GetFence(), presentFenceValue));
  }
#endif

  // Submit updated command list, if needed
  UINT64 uploadFenceValue = NoneValue;
  D3D_CHECK(UploadQueue->CloseAndSubmitCommandList(&uploadFenceValue));

  if (uploadFenceValue != NoneValue) // if we have some updates. 
  {
    // Add fence to pending
    UploadBuffer->AddPendingFence(uploadFenceValue);
  }

  CurrentUploadCmdList = nullptr;
}

bool gdr::device::TransitResourceState(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, UINT subresource)
{
  D3D12_RESOURCE_BARRIER barrier;
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = pResource;
  barrier.Transition.Subresource = subresource;
  barrier.Transition.StateBefore = before;
  barrier.Transition.StateAfter = after;

  pCommandList->ResourceBarrier(1, &barrier);

  return true;
}

void gdr::device::ResizeUpdateBuffer(bool isRuntime)
{
  TermUploadEngine();
  InitUploadEngine(
    (UINT64)1 * (isRuntime ? UsedForCreationParams.RuntimeUploadHeapSizeMb : UsedForCreationParams.LoadtimeUploadHeapSizeMb) * 1024 * 1024,
    (UINT64)1 * UsedForCreationParams.DynamicHeapSizeMb * 1024 * 1024,
    UsedForCreationParams.DynamicDescCount);
}

bool gdr::device::CreateGPUResource(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, GPUResource& resource, const void* pInitialData, size_t initialDataSize)
{
  D3D12MA::ALLOCATION_DESC allocDesc;
  allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
  allocDesc.CustomPool = nullptr;
  allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
  allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;

  HRESULT hr = S_OK;
  if (pInitialData == nullptr)
  {
    D3D_CHECK(D3DGPUMemAllocator->CreateResource(&allocDesc, &desc, initialResourceState, pOptimizedClearValue, &resource.Allocation, __uuidof(ID3D12Resource), (void**)&resource.Resource));
  }
  else
  {
    GDR_ASSERT(CurrentUploadCmdList != nullptr);

    D3D_CHECK(D3DGPUMemAllocator->CreateResource(&allocDesc, &desc, D3D12_RESOURCE_STATE_COMMON, pOptimizedClearValue, &resource.Allocation, __uuidof(ID3D12Resource), (void**)&resource.Resource));
    switch (desc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_BUFFER:
      D3D_CHECK(UpdateBuffer(CurrentUploadCmdList, resource.Resource, pInitialData, initialDataSize));
      break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
      D3D_CHECK(UpdateTexture(CurrentUploadCmdList, resource.Resource, pInitialData, initialDataSize));
      break;
    }

    if (SUCCEEDED(hr) && initialResourceState != D3D12_RESOURCE_STATE_COMMON)
    {
      UploadBarriers.push_back({ resource.Resource, initialResourceState });
    }
  }

  return SUCCEEDED(hr);
}

void gdr::device::ReleaseGPUResource(GPUResource& resource)
{
  D3D_RELEASE(resource.Resource);
  D3D_RELEASE(resource.Allocation);
}

// Compile shader into bytecode
bool gdr::device::CompileShader(LPCTSTR srcFilename, const std::vector<LPCSTR>& defines, const shader_stage& stage, ID3DBlob** ppShaderBinary, std::set<std::tstring>* pIncludes)
{
  std::vector<char> data;
  bool res = ReadFileContent(srcFilename, data);
  if (res)
  {
    std::vector<D3D_SHADER_MACRO> macros;
    for (int i = 0; i < defines.size(); i++)
    {
      macros.push_back({ defines[i], nullptr });
    }
    macros.push_back({ nullptr, nullptr });

    UINT flags = D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;

    if (IsDebugShaders)
    {
      flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    }

    ID3DBlob* pErrMsg = nullptr;

    HRESULT hr = S_OK;

    d3dinclude includeCallback;

    switch (stage)
    {
    case Vertex:
      hr = D3DCompile(data.data(), data.size(), "", macros.data(), &includeCallback, "VS", "vs_5_1", flags, 0, ppShaderBinary, &pErrMsg);
      break;

    case Pixel:
      hr = D3DCompile(data.data(), data.size(), "", macros.data(), &includeCallback, "PS", "ps_5_1", flags, 0, ppShaderBinary, &pErrMsg);
      break;

    case Compute:
      hr = D3DCompile(data.data(), data.size(), "", macros.data(), &includeCallback, "CS", "cs_5_1", flags, 0, ppShaderBinary, &pErrMsg);
      break;
    }
    if (pErrMsg != nullptr)
    {
      if (!SUCCEEDED(hr))
      {
        const char* pMsg = (const char*)pErrMsg->GetBufferPointer();
        OutputDebugStringA(pMsg);
        OutputDebugString(_T("\n"));
      }
      D3D_RELEASE(pErrMsg);
    }
    GDR_ASSERT(SUCCEEDED(hr));

    res = SUCCEEDED(hr);

    if (res && pIncludes != nullptr)
    {
      std::swap(*pIncludes, includeCallback.includeFiles);
    }
  }
  return res;
}

// Create root signature
bool gdr::device::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& rsDesc, ID3D12RootSignature** ppRootSignature)
{
  ID3DBlob* pSignatureBlob = nullptr;
  ID3DBlob* pErrMsg = nullptr;

  HRESULT hr = S_OK;
  D3D_CHECK(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignatureBlob, &pErrMsg));
  if (pErrMsg != nullptr)
  {
    if (!SUCCEEDED(hr))
    {
      const char* pMsg = (const char*)pErrMsg->GetBufferPointer();
      OutputDebugStringA(pMsg);
      OutputDebugString(_T("\n"));
    }
    D3D_RELEASE(pErrMsg);
  }

  D3D_CHECK(D3DDevice->CreateRootSignature(0, pSignatureBlob->GetBufferPointer(), pSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)ppRootSignature));

  D3D_RELEASE(pSignatureBlob);

  return SUCCEEDED(hr);
}

// Create Pipeline state object 
bool gdr::device::CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ID3D12PipelineState** ppPSO)
{
  HRESULT hr = S_OK;
  D3DDevice->CreateGraphicsPipelineState(&psoDesc, __uuidof(ID3D12PipelineState), (void**)ppPSO);
  return SUCCEEDED(hr);
}

// Create Pipeline state object 
bool gdr::device::CreateComputePSO(const D3D12_COMPUTE_PIPELINE_STATE_DESC& psoDesc, ID3D12PipelineState** ppPSO)
{
  HRESULT hr = S_OK;
  D3DDevice->CreateComputePipelineState(&psoDesc, __uuidof(ID3D12PipelineState), (void**)ppPSO);
  return SUCCEEDED(hr);
}

bool gdr::device::ResizeSwapchain(UINT width, UINT height)
{
  if (width == 0 || height == 0)
  {
    return false;
  }

  WaitGPUIdle();

  HRESULT hr = S_OK;
  DXGI_SWAP_CHAIN_DESC desc;
  D3D_CHECK(Swapchain->GetDesc(&desc));
  if (desc.BufferDesc.Width != width || desc.BufferDesc.Height != height)
  {
    UINT count = (UINT)BackBuffers.size();

    TermBackBuffers();

    D3D_CHECK(Swapchain->ResizeBuffers(count, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
    D3D_CHECK(CreateBackBuffers(count));
  }

  return SUCCEEDED(hr);
}

bool gdr::device::QueryTimestamp(ID3D12GraphicsCommandList* pCommandList, const std::function<void(UINT64)>& cb)
{
  UINT64 id = -1;
  UINT query = -1;
  std::function<void(UINT64)> queryCB;
  ring_buffer_result allocRes = QueryBuffer->Alloc(1, id, queryCB, 1);
  GDR_ASSERT(allocRes == ring_buffer_result::Ok);
  if (allocRes == ring_buffer_result::Ok)
  {
    QueryBuffer->At(id) = cb;

    pCommandList->EndQuery(QueryBuffer->GetQueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP, (UINT)id);

    return true;
  }

  return false;
}

UINT64 gdr::device::GetPresentQueueFrequency() const
{
  UINT64 freq = 0;
  HRESULT hr = S_OK;
  D3D_CHECK(PresentQueue->GetQueue()->GetTimestampFrequency(&freq));

  return freq;
}

void gdr::device::WaitGPUIdle()
{
  UINT64 finishedFenceValue = NoneValue;
  PresentQueue->WaitIdle(finishedFenceValue);
  if (finishedFenceValue != NoneValue)
  {
    DynamicBuffer->FlashFenceValue(finishedFenceValue);
    DynamicDescBuffer->FlashFenceValue(finishedFenceValue);
    QueryBuffer->FlashFenceValue(finishedFenceValue);
  }
}