#include "p_header.h"

void gdr::visibility_pass::Initialize(void)
{
  // 1) Load shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/FrustumVisibility.hlsl"), {}, shader_stage::Compute, &FrustumCullComputeShader);

  // 2) Create root signature for frustum compute
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    CD3DX12_DESCRIPTOR_RANGE descr[4] = {};

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

    // UAV set as Descriptor range
    {
      descr[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, (int)uav_registers::opaque_all_command_pool_register);

      params[(int)root_parameters_frustum_indices::opaque_all_commands_pool_index].InitAsDescriptorTable(
        1, &descr[0]);
    }

    {
      descr[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, (int)uav_registers::opaque_frustum_command_pool_register);

      params[(int)root_parameters_frustum_indices::opaque_frustum_command_pool_index].InitAsDescriptorTable(
        1, &descr[1]);
    }

    {
      descr[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, (int)uav_registers::transparent_culled_command_pool_register);

      params[(int)root_parameters_frustum_indices::transparent_culled_command_pool_index].InitAsDescriptorTable(
        1, &descr[2]);
    }

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 0U, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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

  // 4) Fill data for command signature
  {
    // Each command consists of a CBV update and a DrawInstanced call.
  }
}

void gdr::visibility_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // Idea
  // 1) cull every command to 
  // indirect_command_enum::OpaqueAll;
  // indirect_command_enum::OpaqueFrustrumCulled;
  // indirect_command_enum::TransparentsCulled;
  // 2) Depth prepass plus write to texture indices of every object on screen
  // 3) Draw fullscreen rect and fill bool array with correct data
  // 4) Cull with bool array indirect_command_enum::OpaqueFrustrumCulled;

  // For now we can do only first, without any trouble

  // 1) Frustum Culling
  {
    currentCommandList->SetPipelineState(FrustumCullPSO);

    ID3D12DescriptorHeap* pDescriptorHeaps = Render->GetDevice().GetDescriptorHeap();
    currentCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);
    currentCommandList->SetComputeRootSignature(FrustumCullRootSignature);

    // update CPUComputeRootConstants
    if (!Render->Params.IsVisibilityLocked)
      CPUComputeRootConstants.VP = Render->PlayerCamera.GetVP();
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
      (int)root_parameters_frustum_indices::opaque_all_commands_pool_index, // root parameter index
      Render->IndirectSystem->CommandsGPUDescriptor[(int)indirect_command_enum::OpaqueAll]);

    currentCommandList->SetComputeRootDescriptorTable(
      (int)root_parameters_frustum_indices::opaque_frustum_command_pool_index, // root parameter index
      Render->IndirectSystem->CommandsGPUDescriptor[(int)indirect_command_enum::OpaqueFrustrumCulled]);

    currentCommandList->SetComputeRootDescriptorTable(
      (int)root_parameters_frustum_indices::transparent_culled_command_pool_index, // root parameter index
      Render->IndirectSystem->CommandsGPUDescriptor[(int)indirect_command_enum::TransparentsCulled]);

    currentCommandList->Dispatch(static_cast<UINT>(ceil(Render->IndirectSystem->CPUData.size() / float(ComputeThreadBlockSize))), 1, 1);
  }

  // Transit resource state of buffers to INDIRECT_ARGUMENT.
  for (int i = 1; i < (int)indirect_command_enum::TotalBuffers; i++)
  {
    // transit state from indirect argument to Unordered ACCESS
    Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->IndirectSystem->CommandsBuffer[i].Resource,
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  }
}

gdr::visibility_pass::~visibility_pass(void)
{
  FrustumCullRootSignature->Release();
  FrustumCullPSO->Release();
  FrustumCullComputeShader->Release();
}
