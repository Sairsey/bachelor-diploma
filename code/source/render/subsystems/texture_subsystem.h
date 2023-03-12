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
  class textures_subsystem : public resource_pool_subsystem<gdr_texture, gdr_index_types::texture, 0>
  {
  protected:
    void BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList) override;
    void BeforeRemoveJob(gdr_index index) override;

    const int FramesToWait = 10;
    std::vector<std::pair<GPUResource, int>> ResourceToDelete;
  public:
    // Constructor
    textures_subsystem(render* Rnd);

    // If we try to create texture without params -> we should error about it
    gdr_index Add()
    {
      GDR_FAILED("You cannot create a texture without parameters");
      return NONE_INDEX;
    }

    // Load Texture
    gdr_index Add(std::string name, bool isSrgb = false);

    // Handles for CPU and GPU tables
    D3D12_GPU_DESCRIPTOR_HANDLE TextureTableGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE TextureTableCPU;

    std::vector<CD3DX12_STATIC_SAMPLER_DESC> SamplersDescs;

    // Destructor 
    ~textures_subsystem(void);
  };
}