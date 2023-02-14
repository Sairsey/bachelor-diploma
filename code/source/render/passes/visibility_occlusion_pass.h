#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class visibility_occlusion_pass : public base_pass
  {
  private:
    ID3DBlob* ComputeShader; // Culling only objects in frustum

    ID3D12RootSignature* RootSignature; // Culling only objects in frustum
    GDRGPUComputeGlobals ComputeGlobals;

    ID3D12PipelineState* ComputePSO;

    // indices in root_parameters
    enum struct root_parameters_occlusion_indices
    {
      compute_globals_index = 0,              // root parameter for compute_params
      object_transform_pool_index,            // root parameter for buffer with per object transforms
      all_commands_pool_index,                // root parameter for SRV buffer with indirect commands
      hier_depth_index,                       // root parameter for texture with Hier Depth
      opaque_culled_commands_pool_index,      // root parameter for UAV buffer with indirect commands for culled opaque objects
      transparent_culled_commands_pool_index, // root parameter for UAV buffer with indirect commands for culled transparent objects
      total_root_parameters
    };
  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "visibility_occlusion_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Function to call Direct draw shader */
    void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~visibility_occlusion_pass() override;
  };
}
