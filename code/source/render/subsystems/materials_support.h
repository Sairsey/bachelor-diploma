#pragma once
#include "def.h"
#include "../bin/shaders/shared_structures.h"

/* Project namespace */
namespace gdr
{
  // Object system representation class
  class materials_support
  {
  private:
    // pointer to Render
    render* Render;
    std::vector<ObjectMaterial> StoredCopy; // stored copy to check if data has changed
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
  public:
    // default constructor
    materials_support(render* Rnd);

    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    static ObjectMaterial DefaultMaterial();

    std::vector<ObjectMaterial> CPUData;  // data stored in CPU
    GPUResource GPUData; // data stored in GPU

    // desctructor
    ~materials_support();
  };
}
