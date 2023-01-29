#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Material pool representation class
  class materials_subsystem : public resource_pool_subsystem<GDRGPUMaterial, 128 * sizeof(GDRGPUMaterial)>
  {
  protected:
    void BeforeRemoveJob(gdr_index index) override
    {
      if (IsExist(index))
      {
        if (Get(index).ShadeType == MATERIAL_SHADER_COLOR)
        {
          Render->TexturesSystem->Remove(GDRGPUMaterialColorGetColorMapIndex(Get(index)));
        }
        else if (Get(index).ShadeType == MATERIAL_SHADER_PHONG)
        {
          Render->TexturesSystem->Remove(GDRGPUMaterialPhongGetAmbientMapIndex(Get(index)));
          Render->TexturesSystem->Remove(GDRGPUMaterialPhongGetDiffuseMapIndex(Get(index)));
          Render->TexturesSystem->Remove(GDRGPUMaterialPhongGetSpecularMapIndex(Get(index)));
          Render->TexturesSystem->Remove(GDRGPUMaterialPhongGetNormalMapIndex(Get(index)));
        }
        else if (Get(index).ShadeType == MATERIAL_SHADER_COOKTORRANCE_METALNESS)
        {
          Render->TexturesSystem->Remove(GDRGPUMaterialCookTorranceGetAlbedoMapIndex(Get(index)));
          Render->TexturesSystem->Remove(GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(Get(index)));
          Render->TexturesSystem->Remove(GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(Get(index)));
          Render->TexturesSystem->Remove(GDRGPUMaterialCookTorranceGetNormalMapIndex(Get(index)));
        }
        else if (Get(index).ShadeType == MATERIAL_SHADER_COOKTORRANCE_SPECULAR)
        {
          Render->TexturesSystem->Remove(GDRGPUMaterialCookTorranceGetAlbedoMapIndex(Get(index)));
          Render->TexturesSystem->Remove(GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(Get(index)));
          Render->TexturesSystem->Remove(GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(Get(index)));
          Render->TexturesSystem->Remove(GDRGPUMaterialCookTorranceGetNormalMapIndex(Get(index)));
        }
      }
    }
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
      GDRGPUMaterialPhongGetOpacity(mat) = 1.0f;
      GDRGPUMaterialPhongGetShiness(mat) = 30.0f;
      GDRGPUMaterialPhongGetAmbientMapIndex(mat) = NONE_INDEX;
      GDRGPUMaterialPhongGetDiffuseMapIndex(mat) = NONE_INDEX;
      GDRGPUMaterialPhongGetSpecularMapIndex(mat) = NONE_INDEX;
      GDRGPUMaterialPhongGetNormalMapIndex(mat) = NONE_INDEX;
      return Result;
    }
  };
}
