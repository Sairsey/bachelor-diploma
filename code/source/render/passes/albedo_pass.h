#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class albedo_pass : public base_pass
  {
    private:
      // Our shaders
      ID3DBlob* VertexShader;
      ID3DBlob* ComputeShader;
      ID3DBlob* PixelShader;

      // Root signature
      ID3D12RootSignature* RootSignature;
      ID3D12RootSignature* ComputeRootSignature;

      ComputeRootConstants CPUComputeRootConstants;

      // Command signatures
      ID3D12CommandSignature* CommandSignature;

      // Pipeline state object
      ID3D12PipelineState* PSO;
      ID3D12PipelineState* ComputePSO;
      
      // 'b' registers
      enum struct albedo_buffer_registers
      {
        globals_buffer_register = 0, // some globals data like time/matrix VP/ etc.  Supported
        index_buffer_register        // buffer which contains indices in every pool  Supported
      };

      // 't' registers
      enum struct albedo_texture_registers
      {
        command_pool_register = 0,            // pool with indirect commands 
        index_pool_register,                  // pool with indices in every pool    
        object_transform_pool_register,       // pool with transform data           Supported
        material_pool_register,               // pool with material data
        texture_pool_register,                // pool with texture data
      };

      // 'u' registers
      enum struct albedo_uav_registers
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
        total_root_parameters
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

      // index of used UAV
      int OurUAVIndex;
    public:
      /* Function to get name */
      std::string GetName(void) override 
      {
        return "albedo_pass";
      };

      /* Function to Initialize every PSO/InputLayout/Shaders we need */
      void Initialize(void) override;

      /* Function to call compute shader */
      void CallCompute(ID3D12GraphicsCommandList* currentCommandList) override;

      /* Function to sync result of compute shader */
      void SyncCompute(ID3D12GraphicsCommandList* currentCommandList) override;

      /* Function to call Direct draw shader */
      void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;
    
      /* Function to call Indirect draw shader */
      void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

      /* Virtual Destructor */
      ~albedo_pass() override;
  };
}
