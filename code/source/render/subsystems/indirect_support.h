#pragma once
#include "def.h"
#include "../bin/shaders/shared_structures.h"

/* Project namespace */
namespace gdr
{
  // Data structure to match the command signature used for ExecuteIndirect.
#pragma pack(push, 1)
  struct indirect_command
  {
    ObjectIndices Indices;                 // we want to set buffer with indices
    D3D12_VERTEX_BUFFER_VIEW VertexBuffer; // set correct vertex buffer
    D3D12_INDEX_BUFFER_VIEW IndexBuffer; // set correct index buffer
    D3D12_DRAW_INDEXED_ARGUMENTS DrawArguments; // then draw indirect indexed primitive
    byte _pad1[8];
  };
#pragma pack(pop)
  
  // Indirect data representation class
  class indirect_support
  {
  private:
    render* Render; // pointer on Render

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
  public:
    // Constructor
    indirect_support(render* Rnd);

    // Update data stored on GPU
    void UpdateGPUData(void);

    // Destructor 
    ~indirect_support(void);

    std::vector<indirect_command> CPUData;      // data stored in CPU
    // GPU buffer which contains all commands
    GPUResource CommandsSRV;

    // GPU buffers which contains filtered commands
    int CurrentUAV = 0; // current used UAV
    enum { TotalUAV = 1 }; // Total amount of UAVs
    GPUResource CommandsUAV[TotalUAV];                 // Buffer for filtered commands
    GPUResource CommandsUAVReset;                      // Buffer for reset
    UINT CounterOffset;                                 // Offset to Counter
  };
}