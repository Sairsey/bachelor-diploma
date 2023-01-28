#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

// To draw something you need
// 1) Geometry
// 2) (Optional) Transform
// 3) (Optional) Material
// 4) Draw Command. 
// Just add a new draw command and code should work just fine

/* Project namespace */
namespace gdr
{
  // Struct which represent all UAVs with commands
  enum struct indirect_command_pools_enum
  { 
    All,                                  // SRV with all commands
    OpaqueAll,                            // UAV with all Opaque commands
    TransparentAll,                       // UAV with all Transparent commands
    OpaqueFrustrumCulled,                 // UAV with frustum culled commands
    OpaqueCulled,                         // UAV with frustum and occusion culled Opaque commands
    TransparentsCulled,                   // UAV with frustum and occusion culled Transparents
    TotalBuffers,
  }; // Total amount of UAVs

  // Subsystem which do 3 things
  // 1) Store info about "draw calls" we need to execute
  // 2) Prepare indirect_command_pools_enum::All for indirect draw
  // 3) Store and clear for all other buffers used in indirect draw
  class draw_commands_subsystem : public resource_pool_subsystem<GDRGPUIndirectCommand, sizeof(GDRGPUIndirectCommand)>
  {
  private:
    //CPU Descriptors of all commands
    D3D12_CPU_DESCRIPTOR_HANDLE CommandsCPUDescriptor[(int)indirect_command_pools_enum::TotalBuffers];
  protected:
    // Job to do before update
    void BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList) override;
    // Job to do after update
    void AfterUpdateJob(ID3D12GraphicsCommandList* pCommandList) override;
    // Job to do after update of resource state
    void AfterResourceStateUpdateJob(ID3D12GraphicsCommandList* pCommandList, bool IsRender) override;
    
    // Job to do after update of resource state
    void BeforeRemoveJob(gdr_index index) override;

    size_t UAVStoredSize = 0; // Size on which UAV are allocated
  public:
    // Constructor
    draw_commands_subsystem(render* Rnd);

    // Add one element to pool in correct way
    gdr_index Add(gdr_index geometryIndex, gdr_index transformIndex = NONE_INDEX, gdr_index materialIndex = NONE_INDEX, gdr_index boneMappingIndex = NONE_INDEX);

    // Destructor 
    ~draw_commands_subsystem(void);

    // Emulation of Indirect commands pools
    std::vector<gdr_index> DirectCommandPools[(int)indirect_command_pools_enum::TotalBuffers];

    // command signature for indirect
    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[4] = {};
    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};

    // GPU buffers with commands
    D3D12_GPU_DESCRIPTOR_HANDLE CommandsGPUDescriptor[(int)indirect_command_pools_enum::TotalBuffers]; // GPU Descriptors
    GPUResource CommandsBuffer[(int)indirect_command_pools_enum::TotalBuffers];                        // Buffers for commands
    GPUResource CommandsUAVReset;                                                                      // Buffer for Counter reset
    UINT CounterOffset;                                                                                // Offset to Counter
  };
}