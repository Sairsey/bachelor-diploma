#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class tonemap_pass : public base_pass
  {
  private:
    // Our shaders
    ID3DBlob* FirstVertexShader;
    ID3DBlob* FirstPixelShader;
    ID3DBlob* CopyVertexShader;
    ID3DBlob* CopyPixelShader;
    ID3DBlob* LuminanceFinalComputeShader;
    ID3DBlob* TonemapVertexShader;
    ID3DBlob* TonemapPixelShader;

    // fullscreen rect
    GPUResource ScreenVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW ScreenVertexBufferView;

    // Luminance buffer
    GPUResource LuminanceBuffer;
    D3D12_CPU_DESCRIPTOR_HANDLE LuminanceBufferCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE LuminanceBufferGPU;


    // Root signature
    //ID3D12RootSignature* FirstRootSignature; Same as CopyRootSignature
    ID3D12RootSignature* CopyRootSignature;
    ID3D12RootSignature* LuminanceFinalRootSignature;
    ID3D12RootSignature* TonemapRootSignature;

    // Pipeline state object
    ID3D12PipelineState* FirstPSO;
    ID3D12PipelineState* CopyPSO;
    ID3D12PipelineState* LuminanceFinalPSO;
    ID3D12PipelineState* TonemapPSO;

    // 'b' registers
    enum struct tonemap_buffer_registers
    {
      globals_buffer_register = 0, // some globals data like time/matrix VP/ etc.  Supported
      luminance_variables_register,
    };

    // 't' registers
    enum struct tonemap_texture_registers
    {
      texture_register = 0,                // pool with cube texture data
    };

    // 'u' registers
    enum struct tonemap_uav_registers
    {
      luminance_variables_register = 0,                // pool with cube texture data
    };

    // indices in root_parameters
    enum struct root_parameters_copy_indices
    {
      source_texture_index = 0,
      total_root_parameters
    };

    // indices in root_parameters
    enum struct root_parameters_luminance_final_indices
    {
      globals_buffer_index = 0,
      luminance_variables_index,
      source_texture_index,
      total_root_parameters
    };

    // indices in root_parameters
    enum struct root_parameters_tonemap_indices
    {
      globals_buffer_index = 0,
      hdr_texture_index,
      luminance_variables_index,
      total_root_parameters
    };
  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "tonemap_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~tonemap_pass() override;
  };
}
