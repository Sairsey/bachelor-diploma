#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class visibility_pass : public base_pass
  {
  // will draw plane
  private:
    ID3DBlob* FrustumCullComputeShader; // Culling only objects in frustum
    ID3D12RootSignature* FrustumCullRootSignature; // Culling only objects in frustum
    ComputeRootConstants CPUComputeRootConstants;

    ID3D12PipelineState* FrustumCullPSO;

    // Command signatures
    ID3D12CommandSignature* CommandSignature;

    // 'b' registers
    enum struct buffer_registers
    {
      compute_root_constants = 0,
      index_buffer_register        // buffer which contains indices in every pool  Supported
    };

    // 't' registers
    enum struct texture_registers
    {
      all_command_pool_register = 0,        // pool with indirect commands 
      index_pool_register,                  // pool with indices in every pool    
      object_transform_pool_register,       // pool with transform data
      hier_depth_register,                  // pool with hierarchical Z
    };

    // 'u' registers
    // Use indirect_command_enum from OpaqueAll to TransparentsCulled
    enum struct uav_registers
    {
      opaque_all_command_pool_register = 0,        // pool with indirect commands 
      opaque_frustum_command_pool_register,        // pool with indirect commands 
      opaque_culled_command_pool_register,        // pool with indirect commands 
      transparent_culled_command_pool_register
    };

    // indices in root_parameters
    enum struct root_parameters_frustum_indices
    {
      compute_params_index = 0,           // root parameter for albedo_compute_params
      transform_pool_index,               // root parameter for buffer with transforms
      in_commands_pool_index,             // root parameter for SRV buffer with indirect commands
      opaque_all_commands_pool_index,      // root parameter for UAV buffer with indirect commands
      opaque_frustum_command_pool_index,
      transparent_culled_command_pool_index,
      hier_depth_index,
      total_root_parameters
    };
  public:

    /* Function to get name */
    std::string GetName(void) override
    {
      return "visibility_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~visibility_pass() override;
  };
}
