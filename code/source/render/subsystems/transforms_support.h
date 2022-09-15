#pragma once
#include "def.h"
#include "../bin/shaders/shared_structures.h"

/* Project namespace */
namespace gdr
{
  // Transforms data representation class
  class transforms_support
  {
  private:
    render* Render; // pointer on Render
    std::vector<ObjectTransform> StoredCopy; // stored copy to check if data has changed
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
    bool RecalcInverse = false;
  public:
    // Constructor
    transforms_support(render* Rnd);

    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    std::vector<ObjectTransform> CPUData;  // data stored in CPU
    GPUResource GPUData; // data stored in GPU

    // Destructor 
    ~transforms_support(void);
  };
}