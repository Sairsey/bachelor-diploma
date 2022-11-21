#include "p_header.h"

void gdr::frustum_pass::Initialize(void)
{
  // 1) Load shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/FrustumVisibility.hlsl"), {}, shader_stage::Compute, &FrustumCullComputeShader);

  // 2) Create root signature for frustum compute
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    CD3DX12_DESCRIPTOR_RANGE descr[3] = {};

    params.resize((int)root_parameters_frustum_indices::total_root_parameters);

    {
      params[(int)root_parameters_frustum_indices::compute_params_index].InitAsConstants(
        sizeof(ComputeRootConstants) / sizeof(int32_t),
        (int)buffer_registers::compute_root_constants);
    }

    {
      params[(int)root_parameters_frustum_indices::transform_pool_index].InitAsShaderResourceView(
        (int)texture_registers::object_transform_pool_register);
    }

    {
      params[(int)root_parameters_frustum_indices::in_commands_pool_index].InitAsShaderResourceView(
        (int)texture_registers::all_command_pool_register);
    }

    {
      descr[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, (int)uav_registers::opaque_frustum_command_pool_register);

      params[(int)root_parameters_frustum_indices::opaque_frustum_command_pool_index].InitAsDescriptorTable(
        1, &descr[1]);
    }

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &FrustumCullRootSignature);
  }

  // 3) Create frustum compute PSO
  {
    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = FrustumCullRootSignature;
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(FrustumCullComputeShader);

    Render->GetDevice().CreateComputePSO(computePsoDesc, &FrustumCullPSO);
  }
}

void gdr::frustum_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // Idea
  // 1) cull every command to
  // indirect_command_enum::OpaqueFrustrumCulled;
  // 2) use it for Depth prepass
  // 3) Do Hier-Z
  // 4) Cull with Hier-Z all commands

  // In case we have Direct render
  if (!Render->Params.IsIndirect)
  {
    // do nothing
    return;
  }

  // 1) Frustum Culling
  {
    currentCommandList->SetPipelineState(FrustumCullPSO);

    ID3D12DescriptorHeap* pDescriptorHeaps = Render->GetDevice().GetDescriptorHeap();
    currentCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);
    currentCommandList->SetComputeRootSignature(FrustumCullRootSignature);

    // update CPUComputeRootConstants
    if (!Render->Params.IsVisibilityLocked)
    {
      CPUComputeRootConstants.VP = Render->PlayerCamera.GetVP();
      CPUComputeRootConstants.width = Render->GlobalsSystem->CPUData.width;
      CPUComputeRootConstants.height = Render->GlobalsSystem->CPUData.height;
    }
    CPUComputeRootConstants.enableCulling = Render->Params.IsCulling;
    CPUComputeRootConstants.commandCount = (float)Render->IndirectSystem->CPUData.size();

    // set Root constants buffers etc
    currentCommandList->SetComputeRoot32BitConstants(
      (int)root_parameters_frustum_indices::compute_params_index, // root parameter index
      sizeof(ComputeRootConstants) / sizeof(int32_t),
      &CPUComputeRootConstants,
      0);

    currentCommandList->SetComputeRootShaderResourceView(
      (int)root_parameters_frustum_indices::transform_pool_index, // root parameter index
      Render->TransformsSystem->GPUData.Resource->GetGPUVirtualAddress());

    currentCommandList->SetComputeRootShaderResourceView(
      (int)root_parameters_frustum_indices::in_commands_pool_index, // root parameter index
      Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::All].Resource->GetGPUVirtualAddress());

    currentCommandList->SetComputeRootDescriptorTable(
      (int)root_parameters_frustum_indices::opaque_frustum_command_pool_index, // root parameter index
      Render->IndirectSystem->CommandsGPUDescriptor[(int)indirect_command_enum::OpaqueFrustrumCulled]);

    currentCommandList->Dispatch(static_cast<UINT>(ceil(Render->IndirectSystem->CPUData.size() / float(ComputeThreadBlockSize))), 1, 1);
  }

  // Transit resource state of frustum culled buffer to INDIRECT_ARGUMENT.
  Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueFrustrumCulled].Resource,
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

gdr::frustum_pass::~frustum_pass(void)
{
  FrustumCullRootSignature->Release();
  FrustumCullPSO->Release();
  FrustumCullComputeShader->Release();
}
