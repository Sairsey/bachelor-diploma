#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Hierarchical transforms data representation class
  class node_transforms_subsystem : public resource_pool_subsystem<GDRGPUNodeTransform, 64 * sizeof(GDRGPUNodeTransform)>
  {
  private:
    // update all nodes if needed
    void UpdateHierarchy(gdr_index index);
  protected:
    void BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList) override;
  public:
    // Constructor
    node_transforms_subsystem(render* Rnd) : resource_pool_subsystem(Rnd)
    {
      ResourceName = L"Node transforms pool";
      Add(); // Add one debug
    };

    // Custom Add mechanism
    gdr_index Add(gdr_index parent = NONE_INDEX);

    // Get element the way it can be edited
    GDRGPUNodeTransform& GetEditable(gdr_index index)
    {
        if (IsExist(index))
        {
            CPUData[index].IsNeedRecalc = true;
        }
        return resource_pool_subsystem::GetEditable(index);
    }

    // Custom Remove mechanism
    void Remove(gdr_index node);
  };
}