#include "p_header.h"

void gdr::occlusion_pass::Initialize(void)
{
  // 1) Load shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/OcclusionVisibility.hlsl"), {}, shader_stage::Compute, &OcclusionCullComputeShader);

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

    {
      descr[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, (int)texture_registers::hier_depth_register);

      params[(int)root_parameters_frustum_indices::hier_depth_index].InitAsDescriptorTable(
        1, &descr[3]);
    }

    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[1];
    samplerDescs[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_POINT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], sizeof(samplerDescs) / sizeof(samplerDescs[0]), samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &OcclusionCullRootSignature);
  }

  // 3) Create frustum compute PSO
  {
    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = OcclusionCullRootSignature;
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(OcclusionCullComputeShader);

    Render->GetDevice().CreateComputePSO(computePsoDesc, &OcclusionCullPSO);
  }
}

void gdr::occlusion_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // Idea
  // 1) cull every command to 
  // indirect_command_enum::OpaqueAll;
  // indirect_command_enum::OpaqueFrustrumCulled;
  // indirect_command_enum::TransparentsCulled;
  // 2) Depth prepass
  // 3) Hier-Z
  // 4) Cull with Hier-Z indirect_command_enum::OpaqueFrustrumCulled;

  // In case we have Direct render
  
  if (!Render->Params.IsIndirect)
  {
    for (int i = 1; i < (int)indirect_command_enum::TotalBuffers; i++)
    {
      // transit state from indirect argument to Unordered ACCESS
      Render->GetDevice().TransitResourceState(
        currentCommandList,
        Render->IndirectSystem->CommandsBuffer[i].Resource,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    }
    return;
  }

  // 1) Frustum Culling
  {
    currentCommandList->SetPipelineState(OcclusionCullPSO);

    ID3D12DescriptorHeap* pDescriptorHeaps = Render->GetDevice().GetDescriptorHeap();
    currentCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);
    currentCommandList->SetComputeRootSignature(OcclusionCullRootSignature);

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
      (int)root_parameters_frustum_indices::opaque_all_commands_pool_index, // root parameter index
      Render->IndirectSystem->CommandsGPUDescriptor[(int)indirect_command_enum::OpaqueAll]);

    currentCommandList->SetComputeRootDescriptorTable(
      (int)root_parameters_frustum_indices::opaque_frustum_command_pool_index, // root parameter index
      Render->IndirectSystem->CommandsGPUDescriptor[(int)indirect_command_enum::OpaqueFrustrumCulled]);

    currentCommandList->SetComputeRootDescriptorTable(
      (int)root_parameters_frustum_indices::transparent_culled_command_pool_index, // root parameter index
      Render->IndirectSystem->CommandsGPUDescriptor[(int)indirect_command_enum::TransparentsCulled]);

    currentCommandList->SetComputeRootDescriptorTable(
      (int)root_parameters_frustum_indices::hier_depth_index, // root parameter index
      Render->HierDepth->TextureGPUDescriptorHandle);

    currentCommandList->Dispatch(static_cast<UINT>(ceil(Render->IndirectSystem->CPUData.size() / float(ComputeThreadBlockSize))), 1, 1);
  }

  // Transit resource state of buffers to INDIRECT_ARGUMENT.
  for (int i = 1; i < (int)indirect_command_enum::TotalBuffers; i++)
  {
    //if (i == (int)indirect_command_enum::OpaqueCulled)
    //  continue;
    // transit state from indirect argument to Unordered ACCESS
    Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->IndirectSystem->CommandsBuffer[i].Resource,
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  }
}

gdr::occlusion_pass::~occlusion_pass(void)
{
  OcclusionCullRootSignature->Release();
  OcclusionCullPSO->Release();
  OcclusionCullComputeShader->Release();
}
