#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Struct which represent all UAVs with commands
  enum struct indirect_command_pools_enum
  { 
    All,                                  // SRV with all commands
    OpaqueAll,                            // UAV with all Opaque commands
    TransparentAll,                       // UAV with all Transparent commands
    FrustrumCulled,                       // UAV with frustum culled commands
    OpaqueCulled,                         // UAV with frustum and occusion culled Opaque commands
    TransparentsCulled,                   // UAV with frustum and occusion culled Transparents
    TotalBuffers,
  }; // Total amount of UAVs

  // Indirect commands support representation class
  class indirect_subsystem
  {
  public:
  private:
    render* Render; // pointer on Render

    D3D12_CPU_DESCRIPTOR_HANDLE CommandsCPUDescriptor[(int)indirect_command_pools_enum::TotalBuffers];

  public:
    // Constructor
    indirect_subsystem(render* Rnd);

    // Update data stored on GPU
    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    // Destructor 
    ~indirect_subsystem(void);

    std::vector<indirect_subsystem> CPUData;      // data stored in CPU

    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[4] = {};

    // GPU buffer which contains all commands
    D3D12_GPU_DESCRIPTOR_HANDLE CommandsGPUDescriptor[(int)indirect_command_pools_enum::TotalBuffers]; // GPU Descriptors
    GPUResource CommandsBuffer[(int)indirect_command_pools_enum::TotalBuffers];                        // Buffers for commands
    GPUResource CommandsUAVReset;                                                                      // Buffer for Counter reset
    UINT CounterOffset;                                                                                // Offset to Counter
  };
}