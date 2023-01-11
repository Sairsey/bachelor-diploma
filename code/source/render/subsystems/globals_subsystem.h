#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Global data representation class
  class globals_subsystem
  {
    private:
      render *Render; // pointer on Render
      GDRGPUGlobalData StoredCopy; // stored copy to check if data has changed
      D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
      D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
    public:
      // Constructor
        globals_subsystem(render *Rnd);

      // Update data stored on GPU
      void UpdateGPUData(ID3D12GraphicsCommandList *pCommandList);

      // Destructor 
      ~globals_subsystem(void);

      GDRGPUGlobalData CPUData;      // data stored in CPU
      GPUResource GPUData;     // data stored in GPU
  };
}