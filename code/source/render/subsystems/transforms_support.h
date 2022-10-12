#pragma once
#include "def.h"
#include "../bin/shaders/shared_structures.h"

/* Project namespace */
namespace gdr
{
  // Transforms data representation class
  class transforms_support
  {
  private:
    render* Render; // pointer on Render
    size_t StoredSize; // stored copy to check if data has changed
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;

    // transform chunk for update
    const int CHUNK_SIZE = 128 * sizeof(ObjectTransform); // 5Kb approx

    std::vector<bool> ChunkMarkings;

    void UpdateInverseTranspose();

  public:
    // Constructor
    transforms_support(render* Rnd);

    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    std::vector<ObjectTransform> CPUData;  // data stored in CPU
    GPUResource GPUData; // data stored in GPU

    void MarkChunkByTransformIndex(size_t index);

    // Destructor 
    ~transforms_support(void);
  };
}