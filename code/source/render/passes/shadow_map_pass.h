#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class shadow_map_pass : public base_pass
  {
  private:
    ID3DBlob* ComputeShader; // Compute Shader to create draw calls 

    ID3DBlob* VertexShader;  // Only vertex shader used to draw

    // Root signature
    ID3D12RootSignature* RootSignature;
    // Pipeline state object
    ID3D12PipelineState* PSO;

    // Root signature
    ID3D12RootSignature* ComputeRootSignature;
    // Pipeline state object
    ID3D12PipelineState* ComputePSO;

    // Command signatures
    ID3D12CommandSignature* CommandSignature;

    // indices in root_parameters
    enum struct root_parameters_draw_indices
    {
      globals_buffer_index = 0,           // root parameter for global buffer
      index_buffer_index,                 // root parameter for buffer with indices in pools
      object_transform_pool_index,        // root parameter for buffer with per object transforms
      node_transform_pool_index,          // root parameter for buffer with nodes transforms
      bone_mapping_pool_index,            // root parameter for buffer with bone mappings
      total_root_parameters,
    };

  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "shadow_map_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Function to call Direct draw shader */
    void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~shadow_map_pass() override;
  };
}
