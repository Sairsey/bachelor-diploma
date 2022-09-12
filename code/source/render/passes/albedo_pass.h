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

      // Pipeline state object
      ID3D12PipelineState* PSO;
      
      // 'b' registers
      enum albedo_buffer_registers
      {
        globals_buffer_register = 0, // some globals data like time/matrix VP/ etc.
        index_buffer_register        // buffer which contains indices in every pool
      };

      // 't' registers
      enum albedo_texture_registers
      {
        command_pool_register = 0,            // pool with indirect commands 
        index_pool_register,                  // pool with indices in every pool
        object_transform_pool_register,       // pool with transform data
        material_pool_register,               // pool with material data
        texture_pool_register,                // pool with texture data
      };

      // 'u' registers
      enum albedo_uav_registers
      {
        indirect_command_pool_register = 0,   // pool with UAV as result of indirect draw
      };

      // indices in root_parameters
      enum root_parameters_draw_indices
      {
        globals_buffer_index = 0,           // root parameter for global buffer
        index_buffer_index,                 // root parameter for buffer with indices in pools
        transform_pool_index,               // root parameter for buffer with transforms
        material_pool_index,                // root parameter for buffer with materials
        texture_pool_index                  // root parameter for buffer with textures
      };

    public:
      /* Function to get name */
      std::string GetName(void) override 
      {
        return "albedo_pass";
      };

      /* Function to Initialize every PSO/InputLayout/Shaders we need */
      void Initialize(void) override;

      /* Function to call compute shader */
      //void CallCompute(ID3D12GraphicsCommandList* currentCommandList) override;

      /* Function to sync result of compute shader */
      //void SyncCompute(ID3D12GraphicsCommandList* currentCommandList) override;

      /* Function to call Direct draw shader */
      void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;
    
      /* Function to call Indirect draw shader */
      //void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

      /* Virtual Destructor */
      ~albedo_pass() override;
  };
}
