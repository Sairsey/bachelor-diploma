#pragma once
#include "def.h"

// TRANSPARENCY SUPPORT VIA OIT

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class oit_transparent_pass : public base_pass
  {
  private:
    // Our shaders
    ID3DBlob* VertexShader;
    ID3DBlob* PixelShader;
    ID3DBlob* ComposeVertexShader;
    ID3DBlob* ComposePixelShader;
    ID3DBlob* ComputeShader;

    // Root signature
    ID3D12RootSignature* ComposeRootSignature;
    ID3D12RootSignature* RootSignature;
    ID3D12RootSignature* ComputeRootSignature;

    ComputeRootConstants CPUComputeRootConstants;

    // Command signatures
    ID3D12CommandSignature* CommandSignature;

    // Pipeline state object
    ID3D12PipelineState* PSO;
    ID3D12PipelineState* ComposePSO;
    ID3D12PipelineState* ComputePSO;

    // fullscreen rect
    GPUResource ScreenVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW ScreenVertexBufferView;


    // 'b' registers
    enum struct transparent_buffer_registers
    {
      globals_buffer_register = 0, // some globals data like time/matrix VP/ etc.
      index_buffer_register        // buffer which contains indices in every pool
    };

    // 't' registers
    enum struct transparent_texture_registers
    {
      command_pool_register = 0,            // pool with indirect commands
      index_pool_register,                  // pool with indices in every pool
      object_transform_pool_register,       // pool with transform data
      material_pool_register,               // pool with material data
      texture_pool_register,                // pool with texture data
      oit_lists_compose_register,           // pool with OIT Lists
      oit_pool_compose_register,            // pool with OIT Nodes
    };

    // 'u' registers
    enum struct transparent_uav_registers
    {
      indirect_command_pool_register = 0,   // pool with UAV as result of indirect draw
      oit_lists_register,                   // pool with OIT lists
      oit_pool_register,                    // pool with elements from OIT lists
    };

    // indices in root_parameters
    enum struct root_parameters_draw_indices
    {
      globals_buffer_index = 0,           // root parameter for global buffer
      index_buffer_index,                 // root parameter for buffer with indices in pools
      transform_pool_index,               // root parameter for buffer with transforms
      material_pool_index,                // root parameter for buffer with materials
      texture_pool_index,                 // root parameter for buffer with textures
      oit_lists_index,
      oit_pool_index,
      total_root_parameters,
    };

    enum struct root_parameters_compose_indices
    {
      globals_buffer_index = 0,           // root parameter for global buffer
      oit_lists_index,
      oit_pool_index,
      total_root_parameters,
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

    // UAVs for OIT
    GPUResource OITLists;
    GPUResource OITListsClearBuffer;
    GPUResource OITPool;

    D3D12_CPU_DESCRIPTOR_HANDLE OITPoolCPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE OITPoolGPUDescriptor;
    D3D12_CPU_DESCRIPTOR_HANDLE OITListsCPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE OITListsGPUDescriptor;
    UINT CounterOffset;
    UINT CurrentOITListsSize;

    std::vector<OITList> OITListsClearVector;

    void CreateOITLists(void);

    // index of used UAV for indirect
    int OurUAVIndex;
  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "oit_transparent_pass";
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
    ~oit_transparent_pass() override;
  };
}
