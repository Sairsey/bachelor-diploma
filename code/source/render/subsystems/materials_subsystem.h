#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Material pool representation class
  class materials_subsystem : public resource_pool_subsystem<GDRGPUMaterial, 128 * sizeof(GDRGPUMaterial)>
  {
  public:
    // default constructor
    materials_subsystem(render* Rnd) : resource_pool_subsystem(Rnd)
    {
      ResourceName = L"Materials pool";
      UsedResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    };

    gdr_index Add()
    {
      gdr_index Result = resource_pool_subsystem::Add();
      GDRGPUMaterial& mat = GetEditable(Result);
      mat.ShadeType = MATERIAL_SHADER_PHONG;
      GDRGPUMaterialPhongGetAmbient(mat) = mth::vec3f(0.f, 0.f, 0.f);
      GDRGPUMaterialPhongGetDiffuse(mat) = mth::vec3f(0.7f, 0.7f, 0.7f);
      GDRGPUMaterialPhongGetSpecular(mat) = mth::vec3f(0.1f, 0.1f, 0.1f);
      GDRGPUMaterialPhongGetShiness(mat) = 30.0f;
      GDRGPUMaterialPhongGetAmbientMapIndex(mat) = NONE_INDEX;
      GDRGPUMaterialPhongGetDiffuseMapIndex(mat) = NONE_INDEX;
      GDRGPUMaterialPhongGetSpecularMapIndex(mat) = NONE_INDEX;
      GDRGPUMaterialPhongGetNormalMapIndex(mat) = NONE_INDEX;
      return Result;
    }
  };
}
