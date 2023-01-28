#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Light sources pool representation class
  class lights_subsystem : public resource_pool_subsystem<GDRGPULightSource, 16 * sizeof(GDRGPULightSource)>
  {
  protected: 
    // 
    void BeforeRemoveJob(gdr_index index) override
    {
      if (IsExist(index) && GetEditable(index).ObjectTransformIndex != NONE_INDEX)
      {
        Render->ObjectTransformsSystem->Remove(GetEditable(index).ObjectTransformIndex);
        GetEditable(index).ObjectTransformIndex = NONE_INDEX;
      }
    }
  public:
    // default constructor
    lights_subsystem(render* Rnd) : resource_pool_subsystem(Rnd)
    {
      ResourceName = L"Light Sources pool";
      UsedResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
      Add(); // Add one, so pool became valid and we can use it
    };

    gdr_index Add()
    {
      gdr_index Result = resource_pool_subsystem::Add();
      GDRGPULightSource& light = GetEditable(Result);
      light.Color = mth::vec3f(1, 1, 1);
      light.LightSourceType = LIGHT_SOURCE_TYPE_DIRECTIONAL;
      light.ConstantAttenuation = 1.0f;
      light.LinearAttenuation = 0.09f;
      light.QuadricAttenuation = 0.032f;
      light.AngleInnerCone = MTH_D2R * 45.0f / 2.0f;
      light.AngleOuterCone = MTH_D2R * 60.0f / 2.0f;
      light.ObjectTransformIndex = NONE_INDEX;
      return Result;
    }

    void SetTransform(gdr_index LightIndex, gdr_index ObjectTransformIndex)
    {
      if (IsExist(LightIndex))
      {
        if (IsExist(ObjectTransformIndex))
          Render->ObjectTransformsSystem->IncreaseReferenceCount(ObjectTransformIndex);
        GetEditable(LightIndex).ObjectTransformIndex = ObjectTransformIndex;
      }
    }
  };
}
