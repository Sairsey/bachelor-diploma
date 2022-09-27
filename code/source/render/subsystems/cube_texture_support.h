#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  // texture representation struct
  struct cube_texture
  {
    bool IsUsed = false;
    bool IsSrv = false;
    std::string Name;
    GPUResource TextureResource;
  };

  const int MAX_CUBE_TEXTURE_AMOUNT = 6;

  // Transforms data representation class
  class cube_textures_support
  {
  private:
    render* Render; // pointer on Render
  public:
    // Constructor
    cube_textures_support(render* Rnd);

    // Load Texture
    int Load(
      std::string namePosX,
      std::string nameNegX,
      std::string namePosY,
      std::string nameNegY,
      std::string namePosZ,
      std::string nameNegZ);

    // Update data on GPU in case we need it 
    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    // Pool with data
    cube_texture CPUPool[MAX_TEXTURE_AMOUNT];

    // Handles for CPU and GPU tables
    D3D12_GPU_DESCRIPTOR_HANDLE CubeTextureTableGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE CubeTextureTableCPU;

    // Destructor 
    ~cube_textures_support(void);
  };
}