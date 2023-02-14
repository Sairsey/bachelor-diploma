#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class visibility_frustum_pass : public base_pass
  {
  // will draw plane
  private:
    ID3DBlob* ComputeShader; // Culling only objects in frustum

    ID3D12RootSignature* RootSignature; // Culling only objects in frustum
    GDRGPUComputeGlobals ComputeGlobals;

    ID3D12PipelineState* ComputePSO;

    // indices in root_parameters
    enum struct root_parameters_frustum_indices
    {
      compute_globals_index = 0,             // root parameter for albedo_compute_params
      object_transform_pool_index,          // root parameter for buffer with per object transforms
      all_commands_pool_index,              // root parameter for SRV buffer with indirect commands
      opaque_all_commands_pool_index,       // root parameter for UAV buffer with indirect commands for all opaque objects
      transparents_all_commands_pool_index, // root parameter for UAV buffer with indirect commands for all transparent objects
      opaque_frustum_commands_pool_index,   // root parameter for UAV buffer with indirect commands for frustum culled opaque objects
      total_root_parameters
    };
  public:


    /* Function to get name */
    std::string GetName(void) override
    {
      return "visibility_frustum_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Function to call Direct draw shader */
    void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~visibility_frustum_pass() override;
  };
}
