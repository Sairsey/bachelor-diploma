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

  // Struct which represent all UAVs with commands
  enum struct indirect_command_enum
  { 
    All,                                  // SRV with all commands
    OpaqueAll,                            // UAV with all Opaque commands
    OpaqueFrustrumCulled,                 // UAV with culled Opaque commands
    OpaqueCulled = OpaqueFrustrumCulled,  // UAV with culled Opaque commands
    TransparentsCulled,                   // UAV with culled Transparents
    TotalBuffers,
  }; // Total amount of UAVs

  
  // Indirect data representation class
  class indirect_support
  {
  public:
  private:
    render* Render; // pointer on Render

    D3D12_CPU_DESCRIPTOR_HANDLE CommandsCPUDescriptor[(int)indirect_command_enum::TotalBuffers];

  public:
    const int index_buffer_index = 1;                 // root parameter for buffer with indices in pools

    // Constructor
    indirect_support(render* Rnd);

    // Update data stored on GPU
    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    // Destructor 
    ~indirect_support(void);

    std::vector<indirect_command> CPUData;      // data stored in CPU

    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[4] = {};

    // GPU buffer which contains all commands
    D3D12_GPU_DESCRIPTOR_HANDLE CommandsGPUDescriptor[(int)indirect_command_enum::TotalBuffers]; // GPU Descriptors
    GPUResource CommandsBuffer[(int)indirect_command_enum::TotalBuffers];  // Buffer for filtered commands
    GPUResource CommandsUAVReset;                               // Buffer for reset
    UINT CounterOffset;                                         // Offset to Counter
  };
}