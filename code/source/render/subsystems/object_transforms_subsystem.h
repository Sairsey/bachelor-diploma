#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Transforms data representation class
  class object_transforms_subsystem : public resource_pool_subsystem<GDRGPUObjectTransform, gdr_index_types::object_transform, 128 * sizeof(GDRGPUObjectTransform)>
  {
  public:
    // Constructor
    object_transforms_subsystem(render* Rnd) : resource_pool_subsystem(Rnd) 
    {
      ResourceName = L"Object transforms pool";
      Add(); // Add one debug
    };

    gdr_index Add()
    {
      gdr_index Result = resource_pool_subsystem::Add();
      GDRGPUObjectTransform &transform = GetEditable(Result);
      transform.maxAABB = mth::vec3f(0, 0, 0);
      transform.minAABB = mth::vec3f(0, 0, 0);
      transform.Transform = mth::matr::Identity();
      return Result;
    }
  };
}