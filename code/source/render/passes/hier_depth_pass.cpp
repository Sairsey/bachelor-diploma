#include "p_header.h"

void gdr::hier_depth_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/DepthColor.hlsl"), {}, shader_stage::Vertex, &VertexShader);

  // 2) Create root signature 
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);
    CD3DX12_DESCRIPTOR_RANGE bindlessTexturesDesc[1];  // Textures Pool
    CD3DX12_DESCRIPTOR_RANGE cubeBindlessTexturesDesc[1];  // Textures Pool

    params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView((int)albedo_buffer_registers::globals_buffer_register);
    params[(int)root_parameters_draw_indices::index_buffer_index].InitAsConstants(sizeof(ObjectIndices) / sizeof(int32_t), (int)albedo_buffer_registers::index_buffer_register);
    params[(int)root_parameters_draw_indices::transform_pool_index].InitAsShaderResourceView((int)albedo_texture_registers::object_transform_pool_register);

    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
    }
  }

  // 3) Create InputLayout
  static const D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BONES_ID", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BONES_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };

  // 4) Create PSO
  // Describe and create the graphics pipeline state object (PSO).
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, 6 };
    psoDesc.pRootSignature = RootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 0;
    psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &PSO);
  }

  // 6) Create Command signature
  {
    Render->GetDevice().GetDXDevice()->CreateCommandSignature(&Render->IndirectSystem->commandSignatureDesc, RootSignature, IID_PPV_ARGS(&CommandSignature));
  }
}

void gdr::hier_depth_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  if (!Render->Params.IsVisibilityLocked)
    Render->HierDepth->Generate(currentCommandList);
}

void gdr::hier_depth_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
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
  currentCommandList->ExecuteIndirect(
    CommandSignature,
    (UINT)Render->IndirectSystem->CPUData.size(),
    Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueFrustrumCulled].Resource,
    0,
    Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueFrustrumCulled].Resource,
    Render->IndirectSystem->CounterOffset); // stride to counter

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueFrustrumCulled].Resource,
    D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_COPY_DEST);

  currentCommandList->CopyBufferRegion(
    Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueFrustrumCulled].Resource,
    Render->IndirectSystem->CounterOffset,
    Render->IndirectSystem->CommandsUAVReset.Resource,
    0,
    sizeof(UINT));

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueFrustrumCulled].Resource,
    D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

  if (!Render->Params.IsVisibilityLocked)
    Render->HierDepth->Generate(currentCommandList);
}

gdr::hier_depth_pass::~hier_depth_pass(void)
{
  CommandSignature->Release();
  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
}
