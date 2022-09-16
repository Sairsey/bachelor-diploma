#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  // texture representation struct
  struct texture
  {
    bool IsUsed = false;
    bool IsSrv = false;
    std::string Name;
    size_t W;
    size_t H;
    size_t NumOfMips;
    GPUResource TextureResource;
  };

  const int MAX_TEXTURE_AMOUNT = 1024;

  bool IsInited;

  // Transforms data representation class
  class textures_support
  {
  private:
    render* Render; // pointer on Render
  public:
    // Constructor
    textures_support(render* Rnd);

    // Load Texture
    int Load(std::string name);

    // Update data on GPU in case we need it 
    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    // Pool with data
    texture CPUPool[MAX_TEXTURE_AMOUNT];

    // Handles for CPU and GPU tables
    D3D12_GPU_DESCRIPTOR_HANDLE TextureTableGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE TextureTableCPU;

    // Destructor 
    ~textures_support(void);
  };
}