#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Transforms data representation class
  class object_transforms_subsystem
  {
  private:
    render* Render; // pointer on Render
    size_t StoredSize; // stored copy to check if data has changed
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;

    // transform chunk for update
    const int CHUNK_SIZE = 128 * sizeof(GDRGPUObjectTransform); // 5Kb approx

    std::vector<bool> ChunkMarkings;
  public:
    // Constructor
    object_transforms_subsystem(render* Rnd);

    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    std::vector<GDRGPUObjectTransform> CPUData;  // data stored in CPU
    GPUResource GPUData; // data stored in GPU

    void MarkChunkByTransformIndex(size_t index);

    // Destructor 
    ~object_transforms_subsystem(void);
  };
}