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
    void BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList) override
    {
      for (int index = 0; index < AllocatedSize(); index++)
        if (IsExist(index) && Get(index).ShadowMapIndex != NONE_INDEX && Get(index).ObjectTransformIndex != NONE_INDEX)
        {
          if (Get(index).LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
          {
            mth::vec3f loc = Render->ObjectTransformsSystem->Get(Get(index).ObjectTransformIndex).Transform * mth::vec3f(0, 0, 0);
            mth::vec3f lookat = Render->ObjectTransformsSystem->Get(Get(index).ObjectTransformIndex).Transform * mth::vec3f(0, -1, 0);
            float Fov = Get(index).AngleInnerCone;
            float Top = Render->PlayerCamera.GetNear() * std::tan(Fov);
            float Bottom = -Top;
            float Left = -Top;
            float Right = Top;

            // update VP and InvVP
            GetEditable(index).VP = mth::matr4f::View(loc, lookat, {0, 1, 0}) * mth::matr4f::Frustum(Left, Right, Bottom, Top, Render->PlayerCamera.GetNear(), Render->PlayerCamera.GetFar());
            GetEditable(index).InvVP = Get(index).VP.Inversed();
          }
        }
    }

    void BeforeRemoveJob(gdr_index index) override
    {
      if (IsExist(index) && Get(index).ObjectTransformIndex != NONE_INDEX)
      {
        Render->ObjectTransformsSystem->Remove(Get(index).ObjectTransformIndex);
        GetEditable(index).ObjectTransformIndex = NONE_INDEX;
      }
      if (IsExist(index) && Get(index).ShadowMapIndex != NONE_INDEX)
      {
        Render->ShadowMapsSystem->Remove(Get(index).ShadowMapIndex);
        GetEditable(index).ShadowMapIndex = NONE_INDEX;
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
      light.ShadowMapIndex = NONE_INDEX;
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
