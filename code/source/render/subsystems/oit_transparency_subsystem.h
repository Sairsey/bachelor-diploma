#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Global data representation class
  class oit_transparency_subsystem
  {
  private:
    render* Render; // pointer on Render

    int W;
    int H;
    int CounterOffset;

    // Heap with all descriptors
    ID3D12DescriptorHeap* D3DDynamicHeap;
  public:
    // Constructor
    oit_transparency_subsystem(render* Rnd);

    // Update GPU data
    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    // Update state of resource. If "IsRender" is true, then update to UsedResourceState. Otherwise to D3D12_RESOURCE_STATE_COPY_DEST
    void UpdateResourceState(ID3D12GraphicsCommandList* pCommandList, bool IsRender);

    // Destructor 
    ~oit_transparency_subsystem(void);

    D3D12_CPU_DESCRIPTOR_HANDLE NodesPoolUAVCPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE NodesPoolUAVGPUDescriptor;
    GPUResource OITNodesPoolGPUData;                        // data stored in GPU
    D3D12_CPU_DESCRIPTOR_HANDLE NodesPoolSRVCPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE NodesPoolSRVGPUDescriptor;
    
    D3D12_CPU_DESCRIPTOR_HANDLE TextureUAVCPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE TextureUAVGPUDescriptor;
    GPUResource OITTexture;                                 // data stored in GPU
    D3D12_CPU_DESCRIPTOR_HANDLE TextureSRVCPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE TextureSRVGPUDescriptor;
  };
}