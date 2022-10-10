#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class hdr_copy_pass : public base_pass
  {
  private:
    // Our shaders
    ID3DBlob* VertexShader;
    ID3DBlob* PixelShader;

    // fullscreen rect
    GPUResource ScreenVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW ScreenVertexBufferView;

    // Root signature
    ID3D12RootSignature* RootSignature;

    // Pipeline state object
    ID3D12PipelineState* PSO;

    // 't' registers
    enum struct hdr_copy_texture_registers
    {
      hdr_texture_register = 0,                // pool with cube texture data
    };

    // indices in root_parameters
    enum root_parameters_draw_indices
    {
      hdr_texture_index = 0,                  // Shader resource 
      total_root_parameters
    };

  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "hdr_copy_pass";
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
    ~hdr_copy_pass() override;
  };
}
