#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class hier_depth_pass : public base_pass
  {
  private:
    ID3DBlob* VertexShader;

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
    };

    // indices in root_parameters
    enum struct root_parameters_draw_indices
    {
      globals_buffer_index = 0,           // root parameter for global buffer
      index_buffer_index,                 // root parameter for buffer with indices in pools
      transform_pool_index,               // root parameter for buffer with transforms
      total_root_parameters,
    };
  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "hier_depth_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Function to call Indirect draw shader */
    void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~hier_depth_pass() override;
  };
}
