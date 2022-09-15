#pragma once
#include "def.h"
#include "../bin/shaders/shared_structures.h"

/* Project namespace */
namespace gdr
{
  // Global data representation class
  class globals_support
  {
    private:
      render *Render; // pointer on Render
      GlobalData StoredCopy; // stored copy to check if data has changed
      D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
      D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
    public:
      // Constructor
      globals_support(render *Rnd);

      // Update data stored on GPU
      void UpdateGPUData(ID3D12GraphicsCommandList *pCommandList);

      // Destructor 
      ~globals_support(void);

      GlobalData CPUData;      // data stored in CPU
      GPUResource GPUData;     // data stored in GPU
  };
}