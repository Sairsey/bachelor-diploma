#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Hierarchical transforms data representation class
  class node_transforms_subsystem
  {
  private:
    render* Render; // pointer on Render
    size_t StoredSize; // stored copy to check if data has changed
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;

    // transform chunk for update
    const int CHUNK_SIZE = 64 * sizeof(GDRGPUNodeTransform); // 12.75 Kb

    std::vector<bool> ChunkMarkings;

    // update all nodes if needed
    void UpdateHierarchy(gdr_index index);
  public:
    // Constructor
    node_transforms_subsystem(render* Rnd);

    // functions to add and delete Node to hierarchy
    gdr_index CreateNode(gdr_index parent = NONE_INDEX);
    void DeleteNode(gdr_index node);

    gdr_index AddElementInPool()
    {
        return CreateNode(NONE_INDEX);
    }

    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    std::vector<GDRGPUNodeTransform> CPUData;  // data stored in CPU
    GPUResource GPUData; // data stored in GPU

    void MarkChunkByTransformIndex(gdr_index index);

    // Destructor 
    ~node_transforms_subsystem(void);
  };
}