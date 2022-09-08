#pragma once

#include "def.h"
#include "D3D12MemAlloc.h"

struct IDXGISwapChain3;

/* Project namespace */
namespace gdr
{
  struct device_create_params
  {
    bool DebugLayer = false;
    bool DebugShaders = false;
    int  CmdListCount = 2;
    int  UploadListCount = 2;
    HWND hWnd = nullptr;
    int  UploadHeapSizeMb = 16;
    int  DynamicHeapSizeMb = 16;
    int  ReadbackHeapSizeMb = 1;
    int  DynamicDescCount = 16384 * 16;
    int  StaticDescCount = 16384 * 16;
    int  RenderTargetViewCount = 256;
    int  QueryCount = 128;
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
      
      // Pending Barrier representation type
      struct PendingBarrier
      {
        ID3D12Resource* pResource = nullptr; // used resource
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_COMMON; // next state
      };

      // Is device inited correctly
      bool IsInited;
      D3D12MA::Allocator* D3DGPUMemAllocator;

      IDXGIFactory* DxgiFactory;
      IDXGIAdapter* DxgiAdapter;
      ID3D12Device* DxgiDevice;

      ID3D12DescriptorHeap* D3DDescriptorHeap;

      bool IsDebugShaders;
      bool IsDebugDevice;

      UINT RtvDescSize;
      UINT DsvDescSize;
      UINT SrvDescSize;
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

      // Default destructor
      ~device();
  };
}
