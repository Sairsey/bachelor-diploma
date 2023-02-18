#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  // texture representation struct
  struct gdr_shadow_map
  {
    bool IsSrvInited = false;
    bool IsDSVInited = false;
    size_t W;
    size_t H;
    GPUResource TextureResource;
  };

  // Transforms data representation class
  class shadow_maps_subsystem : public resource_pool_subsystem<gdr_shadow_map, 0>
  {
  protected:
    void BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList) override;
    void BeforeRemoveJob(gdr_index index) override;
  public:
    // Constructor
    shadow_maps_subsystem(render* Rnd);

    // If we try to create texture without params -> we should error about it
    gdr_index Add()
    {
      GDR_FAILED("You cannot create a shadowmap without parameters");
      return NONE_INDEX;
    }

    // Create DSV Texture
    gdr_index Add(int W, int H);

    // Handles for CPU and GPU tables
    D3D12_GPU_DESCRIPTOR_HANDLE ShadowMapTableGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE ShadowMapTableCPU;

    // Destructor 
    ~shadow_maps_subsystem(void);
  };
}