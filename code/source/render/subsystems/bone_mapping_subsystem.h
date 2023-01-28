#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Maps BoneIndex from GDRVertex to NodeTransform 
  class bone_mapping_subsystem : public resource_pool_subsystem<GDRGPUBoneMapping, 1 * sizeof(GDRGPUBoneMapping)>
  {
  protected:
    void BeforeRemoveJob(gdr_index index) override
    {
      // iterate over used bones and decrease their ref counter by 1
      for (int i = 0; i < MAX_BONE_PER_MODEL; i++)
      {
        if (Get(index).BoneMapping[i] != NONE_INDEX)
          Render->NodeTransformsSystem->Remove(Get(index).BoneMapping[i]);
      }
    }
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

    UINT GetMapping(gdr_index BoneMappingIndex, gdr_index NodeTransformIndex)
    {
      if (IsExist(BoneMappingIndex))
      {
        UINT MappingIndex = MAX_BONE_PER_MODEL;

        for (int i = 0; i < MAX_BONE_PER_MODEL && MappingIndex == MAX_BONE_PER_MODEL; i++)
          // if we found same or place for new
          if (Get(BoneMappingIndex).BoneMapping[i] == NodeTransformIndex ||
            Get(BoneMappingIndex).BoneMapping[i] == NONE_INDEX)
            MappingIndex = i;
        GDR_ASSERT(MappingIndex != MAX_BONE_PER_MODEL);

        GetEditable(BoneMappingIndex).BoneMapping[MappingIndex] = NodeTransformIndex;
        Render->NodeTransformsSystem->IncreaseReferenceCount(NodeTransformIndex);
        return MappingIndex;
      }
      return NONE_INDEX;
    }
  };
}