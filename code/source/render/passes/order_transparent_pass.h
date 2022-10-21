#pragma once
#include "def.h"

// TRANSPARENCY SUPPORT VIA ORDER

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class order_transparent_pass : public base_pass
  {
  private:
    // Our shaders
    ID3DBlob* VertexShader;
    ID3DBlob* PixelShader;

    // Root signature
    ID3D12RootSignature* RootSignature;

    // Pipeline state object
    ID3D12PipelineState* PSO;

    // 'b' registers
    enum struct transparent_buffer_registers
    {
      globals_buffer_register = 0, // some globals data like time/matrix VP/ etc.  Supported
      index_buffer_register        // buffer which contains indices in every pool  Supported
    };

    // 't' registers
    enum struct transparent_texture_registers
    {
      command_pool_register = 0,            // pool with indirect commands
      index_pool_register,                  // pool with indices in every pool
      object_transform_pool_register,       // pool with transform data
      material_pool_register,               // pool with material data
      texture_pool_register,                // pool with texture data
      light_sources_pool_register,          // pool with light sources data
      cube_texture_pool_register,           // pool with cube texture data
    };

    // 'u' registers
    enum struct transparent_uav_registers
    {
      indirect_command_pool_register = 1,   // pool with UAV as result of indirect draw
    };

    // indices in root_parameters
    enum struct root_parameters_draw_indices
    {
      globals_buffer_index = 0,           // root parameter for global buffer
      index_buffer_index,                 // root parameter for buffer with indices in pools
      transform_pool_index,               // root parameter for buffer with transforms
      material_pool_index,                // root parameter for buffer with materials
      texture_pool_index,                 // root parameter for buffer with textures
      light_sources_pool_index,           // root parameter for buffer with light sources
      cube_texture_pool_index,            // root parameter for cube textures
      total_root_parameters,
    };

    // indices in root_parameters
    enum struct root_parameters_compute_indices
    {
      compute_params_index = 0,           // root parameter for albedo_compute_params
      transform_pool_index,               // root parameter for buffer with transforms
      in_commands_pool_index,             // root parameter for SRV buffer with indirect commands
      out_commands_pool_index,            // root parameter for UAV buffer with indirect commands
      total_root_parameters
    };
  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "order_transparent_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~order_transparent_pass() override;
  };
}
