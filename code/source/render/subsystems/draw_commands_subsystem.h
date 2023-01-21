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
    FrustrumCulled,                       // UAV with frustum culled commands
    OpaqueCulled,                         // UAV with frustum and occusion culled Opaque commands
    TransparentsCulled,                   // UAV with frustum and occusion culled Transparents
    TotalBuffers,
  }; // Total amount of UAVs

  // Subsystem which do 3 things
  // 1) Store info about "draw calls" we need to execute
  // 2) Prepare indirect_command_pools_enum::All for indirect draw
  // 3) Store and clear for all other buffers used in indirect draw
  class draw_commands_subsystem
  {
  public:
  private:
    render* Render; // pointer on Render

    D3D12_CPU_DESCRIPTOR_HANDLE CommandsCPUDescriptor[(int)indirect_command_pools_enum::TotalBuffers];

    size_t SavedSize = -1;
  public:
    // Constructor
    draw_commands_subsystem(render* Rnd);

    // Update data stored on GPU
    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    // Add one element to pool in correct way
    gdr_index AddElementInPool(const geometry& Geom)
    {
      GDRGPUIndirectCommand DrawCommand;
      
      DrawCommand.Indices.ObjectIndex = NONE_INDEX;
      DrawCommand.Indices.ObjectParamsMask = 0;
      DrawCommand.Indices.ObjectTransformIndex = NONE_INDEX;
      DrawCommand.Indices.ObjectMaterialIndex = NONE_INDEX;

      DrawCommand.VertexBuffer = Geom.VertexBufferView;
      DrawCommand.IndexBuffer = Geom.IndexBufferView;

      DrawCommand.DrawArguments.IndexCountPerInstance = Geom.IndexCount;
      DrawCommand.DrawArguments.InstanceCount = 1;
      DrawCommand.DrawArguments.BaseVertexLocation = 0;
      DrawCommand.DrawArguments.StartIndexLocation = 0;
      DrawCommand.DrawArguments.StartInstanceLocation = 0;

      CPUData.push_back(DrawCommand);
      return (gdr_index)(CPUData.size() - 1);
    }

    // Add one element to pool in correct way
    gdr_index AddElementInPool(gdr_index geometryIndex)
    {
      return AddElementInPool(Render->GeometrySystem->CPUData[geometryIndex]);
    }

    // Destructor 
    ~draw_commands_subsystem(void);

    std::vector<GDRGPUIndirectCommand> CPUData;      // data stored in CPU

    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[4] = {};

    // GPU buffer which contains all commands
    D3D12_GPU_DESCRIPTOR_HANDLE CommandsGPUDescriptor[(int)indirect_command_pools_enum::TotalBuffers]; // GPU Descriptors
    GPUResource CommandsBuffer[(int)indirect_command_pools_enum::TotalBuffers];                        // Buffers for commands
    GPUResource CommandsUAVReset;                                                                      // Buffer for Counter reset
    UINT CounterOffset;                                                                                // Offset to Counter
  };
}