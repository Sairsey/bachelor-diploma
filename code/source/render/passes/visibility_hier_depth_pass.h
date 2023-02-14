#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class visibility_hier_depth_pass : public base_pass
  {
  private:
    ID3DBlob* VertexShader;  // Only vertex shader used to draw
    ID3DBlob* PixelShader;  // Only vertex shader used to draw
    ID3DBlob* ComputeShader; // Compute Shader to calc Mips

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

    // indices in root_parameters
    enum struct root_parameters_compute_indices
    {
      mips_params_index = 0, // root parameter for mips parameters
      input_srv_index,       // root parameter for input texture
      output_uav_index,      // root parameter for output texture
      total_root_parameters,
    };

    static const unsigned MAX_AMOUNT_OF_MIPS = 20;

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandles[MAX_AMOUNT_OF_MIPS];
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandles[MAX_AMOUNT_OF_MIPS];
  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "visibility_hier_depth_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    // Generate Hier Depth texture
    void Generate(ID3D12GraphicsCommandList* currentCommandList);

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Function to call Direct draw shader */
    void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~visibility_hier_depth_pass() override;
  };
}
