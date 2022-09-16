#include "p_header.h"

void gdr::albedo_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/SimpleColor.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/SimpleColor.hlsl"), {}, shader_stage::Pixel, &PixelShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/SimpleCull.hlsl"), {}, shader_stage::Compute, &ComputeShader);

  // 2) Create root signature 
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);

    {
      params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView(
        (int)albedo_buffer_registers::globals_buffer_register);
    }

    {
      params[(int)root_parameters_draw_indices::index_buffer_index].InitAsConstants(
        sizeof(ObjectIndices) / sizeof(int32_t),
        (int)albedo_buffer_registers::index_buffer_register);
    }

    {
      params[(int)root_parameters_draw_indices::transform_pool_index].InitAsShaderResourceView(
        (int)albedo_texture_registers::object_transform_pool_register);
    }

    {
      params[(int)root_parameters_draw_indices::material_pool_index].InitAsShaderResourceView(
        (int)albedo_texture_registers::material_pool_register);
    }

    if (params.size() != 0)
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
    }
    else
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
    }
  }

  // 3) Create InputLayout
  static const D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };

  // 4) Create PSO
  // Describe and create the graphics pipeline state object (PSO).
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, 3 };
    psoDesc.pRootSignature = RootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader);
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
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &PSO);
  }

  // 5) Create root signature for compute
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    CD3DX12_DESCRIPTOR_RANGE descr = {};

    params.resize((int)root_parameters_compute_indices::total_root_parameters);

    {
      params[(int)root_parameters_compute_indices::compute_params_index].InitAsConstants(
        sizeof(ComputeRootConstants) / sizeof(int32_t),
        (int)albedo_buffer_registers::globals_buffer_register);
    }

    {
      params[(int)root_parameters_compute_indices::transform_pool_index].InitAsShaderResourceView(
        (int)albedo_texture_registers::object_transform_pool_register);
    }

    {
      params[(int)root_parameters_compute_indices::in_commands_pool_index].InitAsShaderResourceView(
        (int)albedo_texture_registers::command_pool_register);
    }

    // UAV set as Descriptor range
    {
      descr.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, (int)albedo_uav_registers::indirect_command_pool_register);

      params[(int)root_parameters_compute_indices::out_commands_pool_index].InitAsDescriptorTable(
        1, &descr);
    }

    if (params.size() != 0)
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init(params.size(), &params[0], 0U, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &ComputeRootSignature);
    }
    else
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init(0U, nullptr, 0U, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &ComputeRootSignature);
    }
  }

  // 6) Create Command signature
  {
    // Each command consists of a CBV update and a DrawInstanced call.
    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[4] = {};

    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = (int)root_parameters_draw_indices::index_buffer_index; // because Root parameter 0 is descriptor handle
    argumentDescs[0].Constant.Num32BitValuesToSet = sizeof(ObjectIndices) / sizeof(int32_t);

    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
    argumentDescs[1].VertexBuffer.Slot = 0;

    argumentDescs[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;

    argumentDescs[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
    commandSignatureDesc.pArgumentDescs = argumentDescs;
    commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
    commandSignatureDesc.ByteStride = sizeof(indirect_command);

    Render->GetDevice().GetDXDevice()->CreateCommandSignature(&commandSignatureDesc, RootSignature, IID_PPV_ARGS(&CommandSignature));
  }

  // 7) Create compute PSO
  {
    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = ComputeRootSignature;
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(ComputeShader);

    Render->GetDevice().CreateComputePSO(computePsoDesc, &ComputePSO);
  }
}

void gdr::albedo_pass::CallCompute(ID3D12GraphicsCommandList* currentCommandList)
{
  // at first get our UAV
  OurUAVIndex = Render->IndirectSystem->CurrentUAV;
  Render->IndirectSystem->CurrentUAV = (Render->IndirectSystem->CurrentUAV + 1) % Render->IndirectSystem->TotalUAV;
  
  currentCommandList->CopyBufferRegion(
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    Render->IndirectSystem->CounterOffset,
    Render->IndirectSystem->CommandsUAVReset.Resource,
    0,
    sizeof(UINT));
  
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  
  currentCommandList->SetPipelineState(ComputePSO);

  ID3D12DescriptorHeap* pDescriptorHeaps = Render->GetDevice().GetDescriptorHeap();
  currentCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);
  currentCommandList->SetComputeRootSignature(ComputeRootSignature);

  // update CPUComputeRootConstants
  CPUComputeRootConstants.VP = Render->PlayerCamera.GetVP();
  CPUComputeRootConstants.enableCulling = Render->Params.IsCulling;
  CPUComputeRootConstants.commandCount = (float)Render->IndirectSystem->CPUData.size();

  // set Root constants buffers etc
  currentCommandList->SetComputeRoot32BitConstants(
    (int)root_parameters_compute_indices::compute_params_index, // root parameter index
    sizeof(ComputeRootConstants) / sizeof(int32_t),
    &CPUComputeRootConstants, 
    0);

  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_compute_indices::transform_pool_index, // root parameter index
    Render->TransformsSystem->GPUData.Resource->GetGPUVirtualAddress());

  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_compute_indices::in_commands_pool_index, // root parameter index
    Render->IndirectSystem->CommandsSRV.Resource->GetGPUVirtualAddress());

  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_compute_indices::out_commands_pool_index, // root parameter index
    Render->IndirectSystem->CommandsUAVGPUDescriptor[OurUAVIndex]);

  currentCommandList->Dispatch(static_cast<UINT>(ceil(Render->IndirectSystem->CPUData.size() / float(128))), 1, 1);
}

void gdr::albedo_pass::SyncCompute(ID3D12GraphicsCommandList* currentCommandList)
{
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

}

void gdr::albedo_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // Update Globals
  Render->GlobalsSystem->CPUData.CameraPos = Render->PlayerCamera.GetPos();
  Render->GlobalsSystem->CPUData.VP = Render->PlayerCamera.GetVP();

  PROFILE_BEGIN(currentCommandList, "Update globals");
  Render->GetDevice().SetCommandListAsUpload(currentCommandList);
  Render->GlobalsSystem->UpdateGPUData(currentCommandList);
  Render->GetDevice().ClearUploadListReference();
  PROFILE_END(currentCommandList);

  // set common params
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // just iterate for every geometry
  for (int i = 0; i < Render->ObjectSystem->CPUPool.size(); i++)
  {
    geometry &geom = Render->GeometrySystem->CPUPool[i];

    currentCommandList->IASetVertexBuffers(0, 1, &geom.VertexBufferView);
    currentCommandList->IASetIndexBuffer(&geom.IndexBufferView);
    {
      currentCommandList->SetGraphicsRootConstantBufferView(
        (int)root_parameters_draw_indices::globals_buffer_index,
        Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRoot32BitConstants(
        (int)root_parameters_draw_indices::index_buffer_index,
        sizeof(ObjectIndices) / sizeof(int32_t),
        &Render->ObjectSystem->CPUPool[i], 0);
      currentCommandList->SetGraphicsRootShaderResourceView(
        (int)root_parameters_draw_indices::transform_pool_index,
        Render->TransformsSystem->GPUData.Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRootShaderResourceView(
        (int)root_parameters_draw_indices::material_pool_index,
        Render->MaterialsSystem->GPUData.Resource->GetGPUVirtualAddress());
    }

    currentCommandList->DrawIndexedInstanced(geom.IndexCount, 1, 0, 0, 0);
  }
}


void gdr::albedo_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // Update Globals
  Render->GlobalsSystem->CPUData.CameraPos = Render->PlayerCamera.GetPos();
  Render->GlobalsSystem->CPUData.VP = Render->PlayerCamera.GetVP();
 
  PROFILE_BEGIN(currentCommandList, "Update globals");
  Render->GetDevice().SetCommandListAsUpload(currentCommandList);
  Render->GlobalsSystem->UpdateGPUData(currentCommandList);
  Render->GetDevice().ClearUploadListReference();
  PROFILE_END(currentCommandList);

  // set common params
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  currentCommandList->SetGraphicsRootConstantBufferView(
    (int)root_parameters_draw_indices::globals_buffer_index,
    Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
  // root_parameters_draw_indices::index_buffer_index will be set via indirect
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::transform_pool_index,
    Render->TransformsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::material_pool_index,
    Render->MaterialsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->ExecuteIndirect(
      CommandSignature,
      Render->IndirectSystem->CPUData.size(),
      Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
      0,
      Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
      Render->IndirectSystem->CounterOffset); // stride to counter

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_COPY_DEST);
    
}

gdr::albedo_pass::~albedo_pass(void)
{
  ComputePSO->Release();
  CommandSignature->Release();
  ComputeRootSignature->Release();
  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
  ComputeShader->Release();
}
