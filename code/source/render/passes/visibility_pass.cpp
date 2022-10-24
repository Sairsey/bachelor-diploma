#include "p_header.h"

void gdr::visibility_pass::Initialize(void)
{
  // 1) Load shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/FrustumVisibility.hlsl"), {}, shader_stage::Compute, &FrustumCullComputeShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/DepthPrepassVisibility.hlsl"), {}, shader_stage::Vertex, &DepthPrepassVertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/DepthPrepassVisibility.hlsl"), {}, shader_stage::Pixel, &DepthPrepassPixelShader);

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

  // 4) Create root signature for Depth Prepass
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;

    params.resize((int)root_parameters_depth_prepass_indices::total_root_parameters);

    params[(int)root_parameters_depth_prepass_indices::globals_buffer_index].InitAsConstantBufferView((int)buffer_registers::compute_root_constants);
    params[(int)root_parameters_depth_prepass_indices::index_buffer_index].InitAsConstants(sizeof(ObjectIndices) / sizeof(int32_t), (int)buffer_registers::index_buffer_register);
    params[(int)root_parameters_depth_prepass_indices::transform_pool_index].InitAsShaderResourceView((int)texture_registers::object_transform_pool_register);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 0U, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &DepthPrepassRootSignature);
  }

  // 5) Create Input Layout
  static const D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };

  // 4) Create PSO
  // Describe and create the graphics pipeline state object (PSO).
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, 4 };
    psoDesc.pRootSignature = DepthPrepassRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(DepthPrepassVertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(DepthPrepassPixelShader);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Render->RenderTargets->TargetParams[(int)render_targets_enum::target_frame_indices].Format;
    psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
    psoDesc.SampleDesc.Count = 1;
    Render->GetDevice().CreatePSO(psoDesc, &DepthPrepassPSO);
  }

  // 6) Create Command signature
  {
    Render->GetDevice().GetDXDevice()->CreateCommandSignature(&Render->IndirectSystem->commandSignatureDesc, DepthPrepassRootSignature, IID_PPV_ARGS(&CommandSignature));
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

  // In case we have Direct render
  /*
  if (!Render->Params.IsIndirect)
  {
    // Transit resource state of buffers to INDIRECT_ARGUMENT.
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
  */

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

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueCulled].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

  // 2) Draw all Opaque FrustumCulled on screen (Also Z-prepass)
  if (0)
  {
    // Set correct Render Target
    Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_frame_indices);
    // Update Globals
    Render->GlobalsSystem->CPUData.CameraPos = Render->PlayerCamera.GetPos();
    Render->GlobalsSystem->CPUData.VP = Render->PlayerCamera.GetVP();

    PROFILE_BEGIN(currentCommandList, "Update globals");
    Render->GetDevice().SetCommandListAsUpload(currentCommandList);
    Render->GlobalsSystem->UpdateGPUData(currentCommandList);
    Render->GetDevice().ClearUploadListReference();
    PROFILE_END(currentCommandList);

    // set common params
    currentCommandList->SetPipelineState(DepthPrepassPSO);
    currentCommandList->SetGraphicsRootSignature(DepthPrepassRootSignature);
    currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    currentCommandList->SetGraphicsRootConstantBufferView(
      (int)root_parameters_depth_prepass_indices::globals_buffer_index,
      Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
    // root_parameters_draw_indices::index_buffer_index will be set via indirect
    currentCommandList->SetGraphicsRootShaderResourceView(
      (int)root_parameters_depth_prepass_indices::transform_pool_index,
      Render->TransformsSystem->GPUData.Resource->GetGPUVirtualAddress());
    currentCommandList->ExecuteIndirect(
      CommandSignature,
      (UINT)Render->IndirectSystem->CPUData.size(),
      Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueCulled].Resource,
      0,
      Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueCulled].Resource,
      Render->IndirectSystem->CounterOffset); // stride to counter

    // set previous render target
    Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_frame_hdr);
  }

  // Transit resource state of buffers to INDIRECT_ARGUMENT.
  for (int i = 1; i < (int)indirect_command_enum::TotalBuffers; i++)
  {
    if (i == (int)indirect_command_enum::OpaqueCulled)
      continue;
    // transit state from indirect argument to Unordered ACCESS
    Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->IndirectSystem->CommandsBuffer[i].Resource,
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  }
}

gdr::visibility_pass::~visibility_pass(void)
{
  CommandSignature->Release();
  FrustumCullRootSignature->Release();
  FrustumCullPSO->Release();
  FrustumCullComputeShader->Release();
  DepthPrepassVertexShader->Release();
  DepthPrepassPixelShader->Release();
  DepthPrepassRootSignature->Release();
  DepthPrepassPSO->Release();
}
