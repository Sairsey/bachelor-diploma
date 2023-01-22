#pragma once
#include "def.h"
#include "../bin/shaders/shared_structures.h"

/* Project namespace */
namespace gdr
{
  // Global data representation class
  class hier_depth_support
  {
  private:
    render* Render; // pointer on Render

    // 'b' registers
    enum struct buffer_registers
    {
      mip_params_register = 0, // some globals data like time/matrix VP/ etc.  Supported
    };

    // 't' registers
    enum struct texture_registers
    {
      input_srv_register = 0,       // pool with transform data
      output_uav_register,           // root parameter for cube textures
    };

    // indices in root_parameters
    enum struct root_parameters_indices
    {
      mip_params_index = 0,           // root parameter for global buffer
      input_srv_index,                 // root parameter for buffer with indices in pools
      output_uav_index,               // root parameter for buffer with transforms
      total_root_parameters,
    };

    // Mip compute shader
    ID3DBlob* ComputeShader;

    // Root signature
    ID3D12RootSignature* RootSignature;

    // Pipeline state object
    ID3D12PipelineState* PSO;

    static const unsigned MAX_AMOUNT_OF_MIPS = 20;

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandles[2 * MAX_AMOUNT_OF_MIPS];
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandles[2 * MAX_AMOUNT_OF_MIPS];
  public:
    // Constructor
    hier_depth_support(render* Rnd);

    // Generate Hierachical Depth
    void Generate(ID3D12GraphicsCommandList* pCommandList);

    // Hierarchical Depth Texture
    GPUResource Texture;
    D3D12_CPU_DESCRIPTOR_HANDLE TextureCPUDescriptorHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE TextureGPUDescriptorHandle;

    // Destructor 
    ~hier_depth_support(void);
  };
}