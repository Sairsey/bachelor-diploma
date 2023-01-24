#include "p_header.h"

void gdr::skybox_pass::Initialize(void)
{
    // 1) Compile our shaders
    Render->GetDevice().CompileShader(_T("bin/shaders/skybox/skybox.hlsl"), {}, shader_stage::Vertex, &VertexShader);
    Render->GetDevice().CompileShader(_T("bin/shaders/skybox/skybox.hlsl"), {}, shader_stage::Pixel, &PixelShader);

    // 2) Create InputLayout
    static const D3D12_INPUT_ELEMENT_DESC skyboxInputElementDescs[] =
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 3) Create root signature 
    {
        std::vector<CD3DX12_ROOT_PARAMETER> params;
        params.resize((int)root_parameters_draw_indices::total_root_parameters);
        CD3DX12_DESCRIPTOR_RANGE cubeBindlessTexturesDesc[1];  // Cube Textures Pool

        params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView(GDRGPUGlobalDataConstantBufferSlot);

        {
          cubeBindlessTexturesDesc[0].BaseShaderRegister = GDRGPUCubeTexturePoolSlot;
          cubeBindlessTexturesDesc[0].NumDescriptors = (UINT)Render->CreationParams.MaxTextureAmount;
          cubeBindlessTexturesDesc[0].OffsetInDescriptorsFromTableStart = 0;
          cubeBindlessTexturesDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
          cubeBindlessTexturesDesc[0].RegisterSpace = GDRGPUCubeTexturePoolSpace;

          params[(int)root_parameters_draw_indices::cube_texture_pool_index].InitAsDescriptorTable(1, cubeBindlessTexturesDesc);
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
        psoDesc.InputLayout = { skyboxInputElementDescs, _countof(skyboxInputElementDescs) };
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
        psoDesc.RTVFormats[0] = Render->RenderTargetsSystem->TargetParams[(int)render_targets_enum::target_frame_hdr].Format;
        psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
        psoDesc.SampleDesc.Count = 1;

        Render->GetDevice().CreatePSO(psoDesc, &PSO);
    }

    // 5) Create Vertices for fullscreen draw
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
    Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_frame_hdr);

    // set common params
    currentCommandList->SetPipelineState(PSO);
    currentCommandList->SetGraphicsRootSignature(RootSignature);
    currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);
    {
        currentCommandList->SetGraphicsRootConstantBufferView(
            (int)root_parameters_draw_indices::globals_buffer_index,
            Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
        currentCommandList->SetGraphicsRootDescriptorTable(
            (int)root_parameters_draw_indices::cube_texture_pool_index,
            Render->CubeTexturesSystem->CubeTextureTableGPU);
    }

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