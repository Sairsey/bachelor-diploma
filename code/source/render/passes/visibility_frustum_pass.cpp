#include "p_header.h"

void gdr::visibility_frustum_pass::Initialize(void)
{
  // 1) Compile our shader
  Render->GetDevice().CompileShader(_T("bin/shaders/visibility/frustum.hlsl"), {}, shader_stage::Compute, &ComputeShader);

  // 2) Create root signature for frustum compute
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    CD3DX12_DESCRIPTOR_RANGE descr[3] = {};

    params.resize((int)root_parameters_frustum_indices::total_root_parameters);

    params[(int)root_parameters_frustum_indices::compute_globals_index].InitAsConstants(sizeof(GDRGPUComputeGlobals) / sizeof(int32_t), GDRGPUComputeGlobalDataConstantBufferSlot);
    params[(int)root_parameters_frustum_indices::object_transform_pool_index].InitAsShaderResourceView(GDRGPUObjectTransformPoolSlot);
    params[(int)root_parameters_frustum_indices::all_commands_pool_index].InitAsShaderResourceView(GDRGPUAllCommandsPoolSlot);
    
    {
      descr[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUOpaqueAllCommandsPoolSlot);
      params[(int)root_parameters_frustum_indices::opaque_all_commands_pool_index].InitAsDescriptorTable(1, &descr[0]);
    }

    {
      descr[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUTransparentAllCommandsPoolSlot);
      params[(int)root_parameters_frustum_indices::transparents_all_commands_pool_index].InitAsDescriptorTable(1, &descr[1]);
    }

    {
      descr[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUOpaqueFrustumCommandsPoolSlot);
      params[(int)root_parameters_frustum_indices::opaque_frustum_commands_pool_index].InitAsDescriptorTable(1, &descr[2]);
    }

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
  }

  // 3) Create frustum compute PSO
  {
    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = RootSignature;
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(ComputeShader);

    Render->GetDevice().CreateComputePSO(computePsoDesc, &ComputePSO);
  }
}

void gdr::visibility_frustum_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
 
}

void gdr::visibility_frustum_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  currentCommandList->SetPipelineState(ComputePSO);

  ID3D12DescriptorHeap* pDescriptorHeaps = Render->GetDevice().GetDescriptorHeap();
  currentCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);
  currentCommandList->SetComputeRootSignature(RootSignature);

  // update ComputeGlobals
  {
    ComputeGlobals.VP = Render->GlobalsSystem->CPUData.VP;
    ComputeGlobals.width = Render->GlobalsSystem->CPUData.width;
    ComputeGlobals.height = Render->GlobalsSystem->CPUData.height;
    ComputeGlobals.enableCulling = true;
    ComputeGlobals.commandCount = Render->DrawCommandsSystem->CPUData.size();
  }

  currentCommandList->SetComputeRoot32BitConstants(
    (int)root_parameters_frustum_indices::compute_globals_index, // root parameter index
    sizeof(GDRGPUComputeGlobals) / sizeof(int32_t),
    &ComputeGlobals,
    0);
  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_frustum_indices::object_transform_pool_index,
    Render->ObjectTransformsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_frustum_indices::all_commands_pool_index,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::All].Resource->GetGPUVirtualAddress());
  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_frustum_indices::opaque_all_commands_pool_index,
    Render->DrawCommandsSystem->CommandsGPUDescriptor[(int)indirect_command_pools_enum::OpaqueAll]);
  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_frustum_indices::transparents_all_commands_pool_index,
    Render->DrawCommandsSystem->CommandsGPUDescriptor[(int)indirect_command_pools_enum::TransparentAll]);
  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_frustum_indices::opaque_frustum_commands_pool_index,
    Render->DrawCommandsSystem->CommandsGPUDescriptor[(int)indirect_command_pools_enum::OpaqueFrustrumCulled]);

  currentCommandList->Dispatch(static_cast<UINT>(ceil(ComputeGlobals.commandCount / float(GDRGPUComputeThreadBlockSize))), 1, 1);

  // Transit resource state of valid buffers to INDIRECT_ARGUMENT.
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueAll].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::TransparentAll].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueFrustrumCulled].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

gdr::visibility_frustum_pass::~visibility_frustum_pass(void)
{
  RootSignature->Release();
  ComputePSO->Release();
  ComputeShader->Release();
}
