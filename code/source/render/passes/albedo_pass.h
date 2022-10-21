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
      ID3DBlob* PixelShader;

      // Root signature
      ID3D12RootSignature* RootSignature;

      // Command signatures
      ID3D12CommandSignature* CommandSignature;

      // Pipeline state object
      ID3D12PipelineState* PSO;
      
      // 'b' registers
      enum struct albedo_buffer_registers
      {
        globals_buffer_register = 0, // some globals data like time/matrix VP/ etc.  Supported
        index_buffer_register        // buffer which contains indices in every pool  Supported
      };

      // 't' registers
      enum struct albedo_texture_registers
      {
        object_transform_pool_register = 0,       // pool with transform data
        material_pool_register,               // pool with material data
        texture_pool_register,                // pool with texture data
        light_sources_pool_register,          // pool with light sources data
        cube_texture_pool_register,           // root parameter for cube textures
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

    public:
      /* Function to get name */
      std::string GetName(void) override 
      {
        return "albedo_pass";
      };

      /* Function to Initialize every PSO/InputLayout/Shaders we need */
      void Initialize(void) override;

      /* Function to call Direct draw shader */
      void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;
    
      /* Function to call Indirect draw shader */
      void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

      /* Virtual Destructor */
      ~albedo_pass() override;
  };
}
