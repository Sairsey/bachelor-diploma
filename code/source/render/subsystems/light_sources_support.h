#pragma once
#include "def.h"
#include "../bin/shaders/shared_structures.h"

/* Project namespace */
namespace gdr
{
  using gdr_light = long long;

  // Object system representation class
  class light_sources_support
  {
  private:
    // pointer to Render
    render* Render;
    std::vector<LightSource> StoredCopy; // stored copy to check if data has changed
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
  public:
    // default constructor
    light_sources_support(render* Rnd);

    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    gdr_light AddDirectionalLightSource();
    gdr_light AddPointLightSource();
    gdr_light AddSpotLightSource();

    LightSource &GetLight(gdr_light Index);
    ObjectTransform &GetTransform(gdr_light Index);

    std::vector<LightSource> CPUData;  // data stored in CPU
    GPUResource GPUData; // data stored in GPU

    // desctructor
    ~light_sources_support();
  };
}
