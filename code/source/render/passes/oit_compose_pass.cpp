#include "p_header.h"

void gdr::oit_compose_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/transparent/oit_compose.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/transparent/oit_compose.hlsl"), {}, shader_stage::Pixel, &PixelShader);

  // 2) Create InputLayout
  static const D3D12_INPUT_ELEMENT_DESC onScreenInputElementDescs[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };

  // 3) Create root signature 
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);
    D3D12_DESCRIPTOR_RANGE textureDesc[1] = {};
    D3D12_DESCRIPTOR_RANGE poolDesc[1] = {};

    params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView(GDRGPUGlobalDataConstantBufferSlot);
    
    {
      textureDesc[0].BaseShaderRegister = GDRGPUOITTextureSRVSlot;
      textureDesc[0].NumDescriptors = 1;
      textureDesc[0].OffsetInDescriptorsFromTableStart = 0;
      textureDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      textureDesc[0].RegisterSpace = 0;
      params[(int)root_parameters_draw_indices::oit_texture_index].InitAsDescriptorTable(1, textureDesc);
    }

    {
      poolDesc[0].BaseShaderRegister = GDRGPUOITPoolSRVSlot;
      poolDesc[0].NumDescriptors = 1;
      poolDesc[0].OffsetInDescriptorsFromTableStart = 0;
      poolDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      poolDesc[0].RegisterSpace = 0;
      params[(int)root_parameters_draw_indices::oit_pool_index].InitAsDescriptorTable(1, poolDesc);
    }

    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init((UINT)params.size(), &params[0], (UINT)Render->TexturesSystem->SamplersDescs.size(), Render->TexturesSystem->SamplersDescs.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
    }
  }

  // 4) Create PSO
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { onScreenInputElementDescs, _countof(onScreenInputElementDescs) };
    psoDesc.pRootSignature = RootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
    psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Render->RenderTargetsSystem->TargetParams[(int)render_targets_enum::target_frame_hdr].Format;
    psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &PSO);
  }

  // 5) Create Vertices for Fullscreen draw
  {
    float vertices[] = {
      -1, -1, 0, 0, 1,
      -1, 1, 0, 0, 0,
      1, -1, 0, 1, 1,
      1, -1, 0, 1, 1,
      1, 1, 0, 1, 0,
      -1, 1, 0, 0, 0
    };

    ID3D12GraphicsCommandList* commandList;
    Render->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "oit_compose pass create screen mesh");

    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer(5 * sizeof(float) * 6),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        nullptr,
        ScreenVertexBuffer,
        vertices,
        5 * sizeof(float) * 6);
      ScreenVertexBuffer.Resource->SetName(L"oit_compose Screen vertex buffer");
    }
    {
      ScreenVertexBufferView.BufferLocation = ScreenVertexBuffer.Resource->GetGPUVirtualAddress();
      ScreenVertexBufferView.StrideInBytes = (UINT)(5 * sizeof(float));
      ScreenVertexBufferView.SizeInBytes = (UINT)(5 * sizeof(float) * 6);
    }

    PROFILE_END(commandList);
    Render->GetDevice().CloseUploadCommandList();
  }

}

void gdr::oit_compose_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // Transit state to needed one
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->OITTransparencySystem->OITTexture.Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  // Transit state to needed one
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->OITTransparencySystem->OITNodesPoolGPUData.Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  if (!Render->Params.IsTransparent)
    return;

  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_frame_hdr);

  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

  {
    currentCommandList->SetGraphicsRootConstantBufferView(
      (int)root_parameters_draw_indices::globals_buffer_index,
      Render->GlobalsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());

    currentCommandList->SetGraphicsRootDescriptorTable(
      (int)root_parameters_draw_indices::oit_texture_index, 
      Render->OITTransparencySystem->TextureSRVGPUDescriptor);

    currentCommandList->SetGraphicsRootDescriptorTable(
      (int)root_parameters_draw_indices::oit_pool_index,
      Render->OITTransparencySystem->NodesPoolSRVGPUDescriptor);
  }

  currentCommandList->DrawInstanced(6, 1, 0, 0);


}

gdr::oit_compose_pass::~oit_compose_pass(void)
{
  Render->GetDevice().ReleaseGPUResource(ScreenVertexBuffer);

  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
}