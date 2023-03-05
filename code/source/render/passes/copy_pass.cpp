#include "p_header.h"

void gdr::copy_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/copy/copy.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/copy/copy.hlsl"), {}, shader_stage::Pixel, &PixelShader);

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
    D3D12_DESCRIPTOR_RANGE descRanges[1] = {};

    params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView(GDRGPUGlobalDataConstantBufferSlot);

    {
      descRanges[0].BaseShaderRegister = GDRGPUUserShaderResource1Slot;
      descRanges[0].NumDescriptors = 1;
      descRanges[0].OffsetInDescriptorsFromTableStart = 0;
      descRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      descRanges[0].RegisterSpace = 0;
      params[(int)root_parameters_draw_indices::input_texture_index].InitAsDescriptorTable(1, descRanges);
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
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // due to opengl matrices, CULL_MODE_FRONT and CULL_MODE_BACK are swapped
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Render->RenderTargetsSystem->TargetParams[(int)render_targets_enum::target_display].Format;
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
    PROFILE_BEGIN(commandList, "fxaa pass create screen mesh");

    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer(5 * sizeof(float) * 6),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        nullptr,
        ScreenVertexBuffer,
        vertices,
        5 * sizeof(float) * 6);
      ScreenVertexBuffer.Resource->SetName(L"fxaa Screen vertex buffer");
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

void gdr::copy_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_display);

  // set common params
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

  {
    currentCommandList->SetGraphicsRootConstantBufferView(
      (int)root_parameters_draw_indices::globals_buffer_index,
      Render->GlobalsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());

    D3D12_GPU_DESCRIPTOR_HANDLE tmp_descr = Render->RenderTargetsSystem->ShaderResourceViewsGPU;
    tmp_descr.ptr += (int)render_targets_enum::target_frame_final * Render->GetDevice().GetSRVDescSize();
    currentCommandList->SetGraphicsRootDescriptorTable(
      (int)root_parameters_draw_indices::input_texture_index, tmp_descr);
  }

  currentCommandList->DrawInstanced(6, 1, 0, 0);
}

gdr::copy_pass::~copy_pass(void)
{
  Render->GetDevice().ReleaseGPUResource(ScreenVertexBuffer);

  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
}