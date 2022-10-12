#include "p_header.h"

// We pack the UAV counter into the same buffer as the commands rather than create
// a separate 64K resource/heap for it. The counter must be aligned on 4K boundaries,
// so we pad the command buffer (if necessary) such that the counter will be placed
// at a valid location in the buffer.
static UINT AlignForUavCounter(UINT bufferSize)
{
  const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
  return (bufferSize + (alignment - 1)) & ~(alignment - 1);
}

void gdr::skybox_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/SkyboxColor.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/SkyboxColor.hlsl"), {}, shader_stage::Pixel, &PixelShader);

  // 2) Create InputLayout
  static const D3D12_INPUT_ELEMENT_DESC combineInputElementDescs[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };

  // 3) Create root signature 
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);
    CD3DX12_DESCRIPTOR_RANGE cubeBindlessTexturesDesc[1];  // Textures Pool


    params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView((int)skybox_buffer_registers::globals_buffer_register);
    
    {
      cubeBindlessTexturesDesc[0].BaseShaderRegister = (int)skybox_texture_registers::cube_texture_pool_register;
      cubeBindlessTexturesDesc[0].NumDescriptors = MAX_CUBE_TEXTURE_AMOUNT;
      cubeBindlessTexturesDesc[0].OffsetInDescriptorsFromTableStart = 0;
      cubeBindlessTexturesDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      cubeBindlessTexturesDesc[0].RegisterSpace = 2;

      params[(int)root_parameters_draw_indices::cube_texture_pool_index].InitAsDescriptorTable(1, cubeBindlessTexturesDesc);
    }

    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[2];
    samplerDescs[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
    samplerDescs[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_POINT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 2, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
  }

  // 4) Create PSO
  // Describe and create the graphics pipeline state object (PSO).
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { combineInputElementDescs, 1 };
    psoDesc.pRootSignature = RootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Render->RenderTargets->TargetParams[(int)render_targets_enum::target_frame_hdr].Format;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &PSO);
  }

  // 9) Create Vertices for fullscreen draw
  {
    float skyboxVertices[] = {
      // positions          
      -1.0f,  1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      -1.0f,  1.0f, -1.0f,
       1.0f,  1.0f, -1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
       1.0f, -1.0f,  1.0f
    };

    ID3D12GraphicsCommandList* commandList;
    Render->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "skybox pass create screen mesh");

    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer(3 * sizeof(float) * 36),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        nullptr,
        ScreenVertexBuffer,
        skyboxVertices,
        3 * sizeof(float) * 36);
      ScreenVertexBuffer.Resource->SetName(L"Skybox Screen vertex buffer");

    }
    {
      ScreenVertexBufferView.BufferLocation = ScreenVertexBuffer.Resource->GetGPUVirtualAddress();
      ScreenVertexBufferView.StrideInBytes = (UINT)(3 * sizeof(float));
      ScreenVertexBufferView.SizeInBytes = (UINT)(3 * sizeof(float) * 36);
    }

    PROFILE_END(commandList);
    Render->GetDevice().CloseUploadCommandList();
  }
}

void gdr::skybox_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_frame_hdr);
  // Update Globals
  Render->GlobalsSystem->CPUData.CameraPos = Render->PlayerCamera.GetPos();

  mth::matr4f tmpView = Render->PlayerCamera.MatrView;

  tmpView[3][0] = 0;
  tmpView[3][1] = 0;
  tmpView[3][2] = 0;

  Render->GlobalsSystem->CPUData.VP = tmpView * Render->PlayerCamera.MatrProj;

  PROFILE_BEGIN(currentCommandList, "Update globals");
  Render->GetDevice().SetCommandListAsUpload(currentCommandList);
  Render->GlobalsSystem->UpdateGPUData(currentCommandList);
  Render->GetDevice().ClearUploadListReference();
  PROFILE_END(currentCommandList);

  // after we drawn every object, we need to Draw fullscreen  with special shader
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

  currentCommandList->SetGraphicsRootConstantBufferView(
    (int)root_parameters_draw_indices::globals_buffer_index,
    Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());

  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::cube_texture_pool_index,
    Render->CubeTexturesSystem->CubeTextureTableGPU);

  currentCommandList->DrawInstanced(36, 1, 0, 0);
}

gdr::skybox_pass::~skybox_pass(void)
{
  Render->GetDevice().ReleaseGPUResource(ScreenVertexBuffer);
  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
}
