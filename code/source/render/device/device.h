#pragma once

#include "def.h"

struct IDXGISwapChain3;

/* Project namespace */
namespace gdr
{
  // Device creation parameters
  struct device_create_params
  {
    bool DebugLayer = false;
    bool DebugShaders = true;
    int  CmdListCount = 2;
    int  UploadListCount = 2;
    HWND hWnd = nullptr;
    int  UploadHeapSizeMb = 1024;
    int  DynamicHeapSizeMb = 16;
    int  ReadbackHeapSizeMb = 1;
    int  DynamicDescCount = 16384 * 16;
    int  StaticDescCount = 16384 * 16;
    int  RenderTargetViewCount = 256;
    int  QueryCount = 128;
  };

  // Avaliable for compilatior shader stages
  enum shader_stage
  {
    Vertex = 0,
    Pixel,
    Compute
  };

  /* Device representation class */
  class device
  {
    private:
      // Init D3D12 Device
      // ARGUMENTS: (bool) is enable debug 
      bool InitD3D12Device(bool DebugDevice);
      // Terminate D3D12 Device
      void TermD3D12Device(void);

      // Init Command buffer
      // ARGUEMTNS: Amount of command queues and amount of Upload command queues
      bool InitCommandQueue(int Count, int UploadCount);
      // Terminate Command buffer
      void TermCommandQueue(void);

      // Init Swapchain
      // ARGUEMTNS: Amount of swapchains and hWnd
      bool InitSwapchain(int Count, HWND hWnd);
      // Terminate Swapchain
      void TermSwapchain(void);

      // Init GPU Memory Allocator
      bool InitGPUMemoryAllocator(void);
      // Terminate GPU Memory Allocator
      void TermGPUMemoryAllocator(void);

      // Init Descriptor Heap
      // ARGUMENTS: Total Amount of descriptors
      bool InitDescriptorHeap(UINT DescCount);
      // Terminate Descriptor Heap
      void TermDescriptorHeap(void);

      /* Init Upload Engine
       * ARGUMENTS: 
       *        - size of upload heap in bytes
       *             UINT64 UploadHeapSize
       *        - size of Dynamic heap in bytes
       *             UINT64 DynamicHeapSize
       *        - amount of dynamic descriptors
       *             UINT DynamicDescCount
       */
      bool InitUploadEngine(UINT64 UploadHeapSize, UINT64 DynamicHeapSize, UINT DynamicDescCount);
      // Terminate Upload Engine
      void TermUploadEngine(void);

      /* Init RenderTargetViewHeap
       * ARGUMENTS: Amount of render targets
       */
      bool InitRenderTargetViewHeap(UINT Count);
      // Terminate RenderTargetViewHeap
      void TermRenderTargetViewHeap(void);

      /* Init Readback Engine
       * ARGUMENTS: Size of Readback Heap in bytes
       */
      bool InitReadbackEngine(UINT64 ReadbackHeapSize);
      // Terminate Readback Engine
      void TermReadbackEngine(void);

      /* Init Queries
       * ARGUMENTS: Amount of Queries 
       */
      bool InitQueries(UINT Count);
      // Terminate Queries
      void TermQueries(void);
      
      /* Init Back buffers
       * ARGUMENTS: Amount of BackBuffers
       * RETURNS: (HRESULT) return code of creation 
       */
      HRESULT CreateBackBuffers(UINT Count);

      // Terminate back buffers
      void TermBackBuffers();

      // Pending Barrier representation type
      struct PendingBarrier
      {
        ID3D12Resource* pResource = nullptr; // used resource
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_COMMON; // next state
      };

      // Is device inited correctly
      bool IsInited;
      // Memory allocator
      D3D12MA::Allocator* D3DGPUMemAllocator;

      IDXGIFactory* DxgiFactory; // factory for retrieving an adapter
      IDXGIAdapter* DxgiAdapter; // Current graphics adapter
      ID3D12Device* D3DDevice;  // Current device

      // Heap with all descriptors
      ID3D12DescriptorHeap* D3DDescriptorHeap;

      bool IsDebugShaders; // flag if we build debug shaders
      bool IsDebugDevice; //  flag if we in "debug" device mode

      UINT RtvDescSize; // size of render target descriptor
      UINT DsvDescSize; // size of DSV descriptor
      UINT SrvDescSize; // size of SRV descriptor

      // Descriptors of Render Target Views
      ID3D12DescriptorHeap* RenderTargetViews;
      // Maximum amount of Render Target Views
      UINT MaxRTViews;
      // Current Render Target View
      UINT CurrentRTView;

      // Swapchain
      IDXGISwapChain3* Swapchain;

      // Present Queue
      present_command_queue* PresentQueue;
      
      // Upload engine support
      // -->
      // Upload Queue
      upload_command_queue* UploadQueue;
      // State Transition Queue
      command_queue* UploadStateTransitionQueue;
      // Current Upload Queue
      ID3D12GraphicsCommandList* CurrentUploadCmdList;
      // vector of barriers
      std::vector<PendingBarrier> UploadBarriers;
      heap_ring_buffer* UploadBuffer;
      // <--

      // Readback engine support
      // -->
      heap_ring_buffer* ReadbackBuffer;
      // <--

      // Dynamic resources support
      // -->
      heap_ring_buffer* DynamicBuffer;
      descriptor_ring_buffer* DynamicDescBuffer;
      UINT DynamicDescCount;
      // <--
      
      // Queries support
      // -->
      query_ring_buffer* QueryBuffer;
      // <--
      
      // Static descriptor heap
      // -->
      UINT StaticDescCount;
      UINT CurrentStaticDescIndex;
      // <--

      // BackBuffer support
      // -->
      // Vector of allocated BackBuffers
      std::vector<ID3D12Resource*> BackBuffers;
      // Vector of allocated BackBuffers Views
      ID3D12DescriptorHeap* BackBufferViews;
      // Current BackBuffer index
      UINT CurrentBackBufferIdx;
      // Amount of backBuffers
      UINT BackBufferCount;
      // <--

    public:
      
      // Default constructor
      device();

      /* Initialize function
       * ARGUMENTS:
       *      - Engine to init
       *          (engine *) Eng
       * RETURNS: true if success, false otherwise
       */
      bool Init(const device_create_params& params);

      /* Terminate function
       * ARGUMENTS: None
       * RETURNS: None
       */
      void Term(void);

      // Begin command list for rendering
      bool BeginRenderCommandList(ID3D12GraphicsCommandList** ppCommandList, ID3D12Resource** ppBackBuffer, D3D12_CPU_DESCRIPTOR_HANDLE* pBackBufferDesc);
      // Close rendering command list and submit it
      bool CloseSubmitAndPresentRenderCommandList(bool vsync = true);

      // Begin command list for uploading
      bool BeginUploadCommandList(ID3D12GraphicsCommandList** ppCommandList);
      // Update GPU buffer
      HRESULT UpdateBuffer(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pBuffer, const void* pData, size_t dataSize);
      // Update GPU buffer
      HRESULT UpdateBufferOffset(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pBuffer, size_t bufferOffset, const void* pData, size_t dataSize);
      // Update GPU Texture
      HRESULT UpdateTexture(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pTexture, const void* pData, size_t dataSize);
      // Close command list for uploading
      void CloseUploadCommandList();

      // Close command list for uploading
      void CloseUploadCommandListBeforeRenderCommandList();

      // Used for override CurrentUploadCmdList in some special cases
      void SetCommandListAsUpload(ID3D12GraphicsCommandList* pCommandList) { CurrentUploadCmdList = pCommandList;};
      void ClearUploadListReference(void) { CurrentUploadCmdList = nullptr; };
      
      // In case we need huge data to be copied
      void WaitAllUploadLists(void);


      // Change state of resource with resource barrier
      bool TransitResourceState(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

      // Create some resource on GPU
      bool CreateGPUResource(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, GPUResource& resource, const void* pInitialData = nullptr, size_t initialDataSize = 0);
      // Release GPU Resource
      void ReleaseGPUResource(GPUResource& resource);

      // Compile shader into bytecode
      bool CompileShader(LPCTSTR srcFilename, const std::vector<LPCSTR>& defines, const shader_stage& stage, ID3DBlob** ppShaderBinary, std::set<std::tstring>* pIncludes = nullptr);
      // Create root signature
      bool CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& rsDesc, ID3D12RootSignature** ppRootSignature);
      // Create Pipeline state object 
      bool CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ID3D12PipelineState** ppPSO);
      // Create Pipeline state object for compute
      bool CreateComputePSO(const D3D12_COMPUTE_PIPELINE_STATE_DESC& psoDesc, ID3D12PipelineState** ppPSO);

      // Resize backbuffers size
      bool ResizeSwapchain(UINT width, UINT height);

      // Allocate Descriptors
      bool AllocateStaticDescriptors(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE& cpuStartHandle, D3D12_GPU_DESCRIPTOR_HANDLE& gpuStartHandle);
      bool AllocateRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, UINT count = 1);

      // Check if module initialized
      inline bool IsInitialized() const { return IsInited; }
      // Get D3D Device
      inline ID3D12Device* GetDXDevice() const { return D3DDevice; }
      inline IDXGIAdapter* GetAdapter() const { return DxgiAdapter; }
      inline ID3D12DescriptorHeap* GetDescriptorHeap() const { return D3DDescriptorHeap;}

      // Get size of SRV
      inline UINT GetSRVDescSize() const { return SrvDescSize; }
      // Get size of DSV
      inline UINT GetDSVDescSize() const { return DsvDescSize; }
      // Get size of Render target view
      inline UINT GetRTVDescSize() const { return RtvDescSize; }

      // Wait GPU for Idle
      void WaitGPUIdle();

      bool QueryTimestamp(ID3D12GraphicsCommandList* pCommandList, const std::function<void(UINT64)>& cb);
      UINT64 GetPresentQueueFrequency() const;

      // Default destructor
      ~device();
  };
}
