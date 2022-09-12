#include "p_header.h"

void gdr::albedo_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/SimpleColor.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/SimpleColor.hlsl"), {}, shader_stage::Pixel, &PixelShader);

  // 2) Create root signature 
  std::vector<D3D12_ROOT_PARAMETER> params;

  {
    CD3DX12_ROOT_PARAMETER param;
    param.InitAsConstantBufferView(albedo_buffer_registers::globals_buffer_register);
    params.push_back(param);
  }

  {
    CD3DX12_ROOT_PARAMETER param;
    param.InitAsConstants(
      sizeof(ObjectIndices) / sizeof(int32_t),
      albedo_buffer_registers::index_buffer_register);
    params.push_back(param);
  }

  if (params.size() != 0)
  {
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
  }
  else
  {
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
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

void gdr::albedo_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
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
      root_parameters_draw_indices::globals_buffer_index,
      Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
    currentCommandList->SetGraphicsRoot32BitConstants(
      root_parameters_draw_indices::index_buffer_index, 
      sizeof(ObjectIndices) / sizeof(int32_t),
      &Render->ObjectSystem->CPUPool[i], 0);
    }

    currentCommandList->DrawIndexedInstanced(geom.IndexCount, 1, 0, 0, 0);
  }
}

gdr::albedo_pass::~albedo_pass(void)
{
  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
}
