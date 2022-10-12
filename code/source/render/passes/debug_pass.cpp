#include "p_header.h"

void gdr::debug_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/DebugColor.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/DebugColor.hlsl"), {}, shader_stage::Pixel, &PixelShader);

  // 2) Create root signature 
  std::vector<D3D12_ROOT_PARAMETER> params;

  {
    CD3DX12_ROOT_PARAMETER param;
    param.InitAsConstantBufferView(debug_buffer_registers::globals_buffer_register);
    params.push_back(param);
  }

  {
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
  }

  // 3) Create InputLayout
  static const D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };

  // 4) Create PSO
  // Describe and create the graphics pipeline state object (PSO).
  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  psoDesc.InputLayout = { inputElementDescs, 1 };
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
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
  psoDesc.NumRenderTargets = 1;
  psoDesc.RTVFormats[0] = Render->RenderTargets->TargetParams[(int)render_targets_enum::target_frame_hdr].Format;
  psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
  psoDesc.SampleDesc.Count = 1;

  Render->GetDevice().CreatePSO(psoDesc, &PSO);

  // 5) Create our geometry
  std::vector<mth::vec3f> Vertices;
  std::vector<UINT32> Indices;
  int gridCells = 10;
  float gridCellSize = 1.0f;
  
  Vertices.resize((gridCells + 1) * 4);
  Indices.resize((gridCells + 1) * 4);
  IndexCount = (gridCells + 1) * 4;
  for (int i = 0; i <= gridCells; i++)
  {
    Vertices[i * 2 + 0] = mth::vec3f{ (-gridCells / 2 + i) * gridCellSize, 0.0f, -gridCells / 2 * gridCellSize };
    Vertices[i * 2 + 1] = mth::vec3f{ (-gridCells / 2 + i) * gridCellSize, 0.0f,  gridCells / 2 * gridCellSize };
    Vertices[(gridCells + 1) * 2 + i * 2 + 0] = mth::vec3f{ -gridCells / 2 * gridCellSize, 0.0f, (-gridCells / 2 + i) * gridCellSize };
    Vertices[(gridCells + 1) * 2 + i * 2 + 1] = mth::vec3f{ gridCells / 2 * gridCellSize, 0.0f, (-gridCells / 2 + i) * gridCellSize };
  }
  for (int i = 0; i < (gridCells + 1) * 4; i++)
  {
    Indices[i] = (UINT16)i;
  }
  ID3D12GraphicsCommandList* commandList;
  Render->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "debug_pass create plane");

  {
    Render->GetDevice().CreateGPUResource(
      CD3DX12_RESOURCE_DESC::Buffer(3 * sizeof(float) * Vertices.size()),
      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
      nullptr,
      VertexBuffer,
      &Vertices[0],
      3 * sizeof(float) * Vertices.size());
    VertexBuffer.Resource->SetName(L"Debug Vertex buffer");
  }
  {
    VertexBufferView.BufferLocation = VertexBuffer.Resource->GetGPUVirtualAddress();
    VertexBufferView.StrideInBytes = (UINT)(3 * sizeof(float));
    VertexBufferView.SizeInBytes = (UINT)(3 * sizeof(float) * Vertices.size());
  }
  {
    Render->GetDevice().CreateGPUResource(
      CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT32) * Indices.size()),
      D3D12_RESOURCE_STATE_INDEX_BUFFER,
      nullptr,
      IndexBuffer,
      &Indices[0],
      sizeof(UINT32) * Indices.size());
    IndexBuffer.Resource->SetName(L"Debug Index buffer");
  }
  {
    IndexBufferView.BufferLocation = IndexBuffer.Resource->GetGPUVirtualAddress();
    IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    IndexBufferView.SizeInBytes = (UINT)(sizeof(UINT32) * Indices.size());
  }

  PROFILE_END(commandList);
  Render->GetDevice().CloseUploadCommandList();
}

void gdr::debug_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_frame_hdr);
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
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

  // just draw our plane every geometry
  {
    currentCommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
    currentCommandList->IASetIndexBuffer(&IndexBufferView);
    {
      currentCommandList->SetGraphicsRootConstantBufferView(
        root_parameters_draw_indices::globals_buffer_index,
        Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
    }

    currentCommandList->DrawIndexedInstanced(IndexCount, 1, 0, 0, 0);
  }
}

gdr::debug_pass::~debug_pass(void)
{
  Render->GetDevice().ReleaseGPUResource(VertexBuffer);
  Render->GetDevice().ReleaseGPUResource(IndexBuffer);
  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
}
