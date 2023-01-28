#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Maps BoneIndex from GDRVertex to NodeTransform 
  class bone_mapping_subsystem : public resource_pool_subsystem<GDRGPUBoneMapping, 1 * sizeof(GDRGPUBoneMapping)>
  {
  public:
    // Constructor
    bone_mapping_subsystem(render* Rnd) : resource_pool_subsystem(Rnd)
    {
      ResourceName = L"Bone Mapping System";
      Add(); // Add one debug
    };

    gdr_index Add()
    {
      gdr_index Result = resource_pool_subsystem::Add();
      GDRGPUBoneMapping &result = GetEditable(Result);
      for (int i = 0; i < MAX_BONE_PER_MODEL; i++)
        result.BoneMapping[i] = NONE_INDEX;
      return Result;
    }
  };
}