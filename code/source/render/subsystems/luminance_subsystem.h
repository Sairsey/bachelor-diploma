#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Global data representation class
  class luminance_subsystem
  {
    private:
      render *Render; // pointer on Render

      bool IsInited = false;
    public:
      // Constructor
      luminance_subsystem(render *Rnd);

      // Update GPU data
      void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

      // Update state of resource. If "IsRender" is true, then update to UsedResourceState. Otherwise to D3D12_RESOURCE_STATE_COPY_DEST
      void UpdateResourceState(ID3D12GraphicsCommandList* pCommandList, bool IsRender);

      // Destructor 
      ~luminance_subsystem(void);

      D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
      D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
      GPUResource GPUData;                        // data stored in GPU
  };
}