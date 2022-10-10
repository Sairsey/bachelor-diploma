#include "p_header.h"

void gdr::hdr_copy_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/Copy.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/Copy.hlsl"), {}, shader_stage::Pixel, &PixelShader);

  // 2) Create InputLayout
  static const D3D12_INPUT_ELEMENT_DESC combineInputElementDescs[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };

  // 3) Create root signature 
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);
    D3D12_DESCRIPTOR_RANGE descRanges[1] = {};

    {
      descRanges[0].BaseShaderRegister = 0;
      descRanges[0].NumDescriptors = 1;
      descRanges[0].OffsetInDescriptorsFromTableStart = 0;
      descRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      descRanges[0].RegisterSpace = 0;
    }

    params[(int)root_parameters_draw_indices::hdr_texture_index].InitAsDescriptorTable(1, descRanges);

    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[1];
    samplerDescs[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 1, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
  }

  // 4) Create PSO
  // Describe and create the graphics pipeline state object (PSO).
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { combineInputElementDescs, 2 };
    psoDesc.pRootSignature = RootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Render->RenderTargets->Formats[(int)render_targets_enum::target_display];
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &PSO);
  }

  // 9) Create Vertices for fullscreen draw
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
    PROFILE_BEGIN(commandList, "hdr_copy pass create screen mesh");

    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer(5 * sizeof(float) * 6),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        nullptr,
        ScreenVertexBuffer,
        vertices,
        5 * sizeof(float) * 6);
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

void gdr::hdr_copy_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_display);

  // after we drawn every object, we need to Draw fullscreen  with special shader
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

  D3D12_GPU_DESCRIPTOR_HANDLE tmp_descr = Render->RenderTargets->ShaderResourceViewsGPU;

  tmp_descr.ptr += (int)render_targets_enum::target_frame * Render->GetDevice().GetSRVDescSize();

  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::hdr_texture_index, tmp_descr);

  currentCommandList->DrawInstanced(6, 1, 0, 0);
}

gdr::hdr_copy_pass::~hdr_copy_pass(void)
{
  Render->GetDevice().ReleaseGPUResource(ScreenVertexBuffer);
  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
}
