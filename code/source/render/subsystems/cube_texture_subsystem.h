#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  // texture representation struct
  struct gdr_cube_texture
  {
    bool IsSrvInited = false;
    std::string Name;
    GPUResource TextureResource;
  };

  // Cube textures data representation class
  class cube_textures_subsystem : public resource_pool_subsystem<gdr_cube_texture, 0>
  {
  protected:
    void BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList) override;
    void BeforeRemoveJob(gdr_index index) override;
  public:
    // Constructor
    cube_textures_subsystem(render* Rnd);

    // If we try to create texture without params -> we should error about it
    gdr_index Add()
    {
      GDR_FAILED("You cannot create a cube texture without parameters");
      return NONE_INDEX;
    }

    // Load Texture (OpenGL-style !!!)
    gdr_index Add(
      std::string namePosX, 
      std::string nameNegX,
      std::string namePosY,
      std::string nameNegY,
      std::string namePosZ,
      std::string nameNegZ);

    // Load Texture (OpenGL-style !!!)
    gdr_index Add(std::string directory, int MipsAmount);

    // Handles for CPU and GPU tables
    D3D12_GPU_DESCRIPTOR_HANDLE CubeTextureTableGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE CubeTextureTableCPU;

    // Destructor 
    ~cube_textures_subsystem(void);
  };
}