#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class debug_pass : public base_pass
  {
    // will draw plane
  private:
    // Our shaders
    ID3DBlob* VertexShader;
    ID3DBlob* PixelShader;

    // Vertices and Indices
    GPUResource VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
    GPUResource IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
    UINT IndexCount;

    // Root signature
    ID3D12RootSignature* RootSignature;

    // Pipeline state object
    ID3D12PipelineState* PSO;

    // 'b' registers
    enum debug_buffer_registers
    {
      globals_buffer_register = 0, // some globals data like time/matrix VP/ etc.  Supported
    };

    // indices in root_parameters
    enum root_parameters_draw_indices
    {
      globals_buffer_index = 0,           // root parameter for global buffer
    };

  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "debug_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Function to call Indirect draw shader */
    //void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~debug_pass() override;
  };
}
