#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  // texture representation struct
  struct gdr_texture
  {
    bool IsSrvInited = false;
    std::string Name;
    size_t W;
    size_t H;
    size_t NumOfMips;
    bool IsTransparent = false;
    GPUResource TextureResource;
  };

  // Transforms data representation class
  class textures_subsystem
  {
  private:
    render* Render; // pointer on Render
  public:
    // Constructor
    textures_subsystem(render* Rnd);

    // Load Texture
    gdr_index AddElementInPool(std::string name, bool isSrgb = false);

    // Update data on GPU in case we need it 
    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    // Pool with data
    std::vector<gdr_texture> CPUData;

    // Handles for CPU and GPU tables
    D3D12_GPU_DESCRIPTOR_HANDLE TextureTableGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE TextureTableCPU;

    std::vector<CD3DX12_STATIC_SAMPLER_DESC> SamplersDescs;

    // Destructor 
    ~textures_subsystem(void);
  };
}