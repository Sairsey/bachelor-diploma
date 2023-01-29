#include "p_header.h"

void gdr::oit_color_pass::Initialize(void)
{
    // 1) Compile our shaders
    Render->GetDevice().CompileShader(_T("bin/shaders/transparent/oit_color.hlsl"), {}, shader_stage::Vertex, &VertexShader);
    Render->GetDevice().CompileShader(_T("bin/shaders/transparent/oit_color.hlsl"), {}, shader_stage::Pixel, &PixelShader);

    // 2) Create root signature 
    {
        std::vector<CD3DX12_ROOT_PARAMETER> params;
        params.resize((int)root_parameters_draw_indices::total_root_parameters);
        CD3DX12_DESCRIPTOR_RANGE bindlessTexturesDesc[1];  // Textures Pool
        CD3DX12_DESCRIPTOR_RANGE cubeBindlessTexturesDesc[1];  // Cube Textures Pool
        CD3DX12_DESCRIPTOR_RANGE oitTextureDesc[1];
        CD3DX12_DESCRIPTOR_RANGE oitPoolDesc[1];

        params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView(GDRGPUGlobalDataConstantBufferSlot);
        params[(int)root_parameters_draw_indices::enviroment_buffer_index].InitAsConstantBufferView(GDRGPUEnviromentConstantBufferSlot);
        params[(int)root_parameters_draw_indices::index_buffer_index].InitAsConstants(sizeof(GDRGPUObjectIndices) / sizeof(int32_t), (int)GDRGPUObjectIndicesConstantBufferSlot);
        static_assert(GDRGPUObjectIndicesRecordRootIndex == (int)root_parameters_draw_indices::index_buffer_index, "Index buffer not in right root signature index");
        params[(int)root_parameters_draw_indices::object_transform_pool_index].InitAsShaderResourceView(GDRGPUObjectTransformPoolSlot);
        params[(int)root_parameters_draw_indices::node_transform_pool_index].InitAsShaderResourceView(GDRGPUNodeTransformPoolSlot);
        params[(int)root_parameters_draw_indices::material_pool_index].InitAsShaderResourceView(GDRGPUMaterialPoolSlot);
        params[(int)root_parameters_draw_indices::lights_pool_index].InitAsShaderResourceView(GDRGPULightsPoolSlot);
        params[(int)root_parameters_draw_indices::bone_mapping_pool_index].InitAsShaderResourceView(GDRGPUBoneMappingSlot);

        {
          bindlessTexturesDesc[0].BaseShaderRegister = GDRGPUTexturePoolSlot;
          bindlessTexturesDesc[0].NumDescriptors = (UINT)Render->CreationParams.MaxTextureAmount;
          bindlessTexturesDesc[0].OffsetInDescriptorsFromTableStart = 0;
          bindlessTexturesDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
          bindlessTexturesDesc[0].RegisterSpace = GDRGPUTexturePoolSpace;

          params[(int)root_parameters_draw_indices::texture_pool_index].InitAsDescriptorTable(1, bindlessTexturesDesc);
        }

        {
          cubeBindlessTexturesDesc[0].BaseShaderRegister = GDRGPUCubeTexturePoolSlot;
          cubeBindlessTexturesDesc[0].NumDescriptors = (UINT)Render->CreationParams.MaxTextureAmount;
          cubeBindlessTexturesDesc[0].OffsetInDescriptorsFromTableStart = 0;
          cubeBindlessTexturesDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
          cubeBindlessTexturesDesc[0].RegisterSpace = GDRGPUCubeTexturePoolSpace;

          params[(int)root_parameters_draw_indices::cube_texture_pool_index].InitAsDescriptorTable(1, cubeBindlessTexturesDesc);
        }

        {
          oitTextureDesc[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUOITTextureUAVSlot);
          params[(int)root_parameters_draw_indices::oit_texture_index].InitAsDescriptorTable(1, oitTextureDesc);
        }

        {
          oitPoolDesc[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUOITPoolUAVSlot);
          params[(int)root_parameters_draw_indices::oit_pool_index].InitAsDescriptorTable(1, oitPoolDesc);
        }

        {
            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init((UINT)params.size(), &params[0], (UINT)Render->TexturesSystem->SamplersDescs.size(), Render->TexturesSystem->SamplersDescs.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
        }
    }

    // 3) Create PSO
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { defaultInputElementLayout, _countof(defaultInputElementLayout) };
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

        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 0;
        psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
        psoDesc.SampleDesc.Count = 1;

        Render->GetDevice().CreatePSO(psoDesc, &PSO);
    }
    // 4) Create Command signature
    Render->GetDevice().GetDXDevice()->CreateCommandSignature(&Render->DrawCommandsSystem->commandSignatureDesc, RootSignature, IID_PPV_ARGS(&CommandSignature));
}

void gdr::oit_color_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
    if (!Render->Params.IsTransparent)
      return;

    Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_frame_hdr);

    // set common params
    currentCommandList->SetPipelineState(PSO);
    currentCommandList->SetGraphicsRootSignature(RootSignature);
    currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // just iterate for every draw call
    for (auto& i : Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::TransparentsCulled])
    {
        if (!Render->DrawCommandsSystem->IsExist(i))
          continue;
        auto &command = Render->DrawCommandsSystem->Get(i);
        currentCommandList->IASetVertexBuffers(0, 1, &command.VertexBuffer);
        currentCommandList->IASetIndexBuffer(&command.IndexBuffer);
        {
            currentCommandList->SetGraphicsRootConstantBufferView(
                (int)root_parameters_draw_indices::globals_buffer_index,
                Render->GlobalsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRootConstantBufferView(
                (int)root_parameters_draw_indices::enviroment_buffer_index,
                Render->EnviromentSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRoot32BitConstants(
                (int)root_parameters_draw_indices::index_buffer_index,
                sizeof(GDRGPUObjectIndices) / sizeof(int32_t),
                &command.Indices, 0);
            currentCommandList->SetGraphicsRootShaderResourceView(
                (int)root_parameters_draw_indices::object_transform_pool_index,
                Render->ObjectTransformsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRootShaderResourceView(
                (int)root_parameters_draw_indices::node_transform_pool_index,
                Render->NodeTransformsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRootShaderResourceView(
                (int)root_parameters_draw_indices::material_pool_index,
                Render->MaterialsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRootShaderResourceView(
              (int)root_parameters_draw_indices::lights_pool_index,
              Render->LightsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRootDescriptorTable(
                (int)root_parameters_draw_indices::texture_pool_index,
                Render->TexturesSystem->TextureTableGPU);
            currentCommandList->SetGraphicsRootDescriptorTable(
              (int)root_parameters_draw_indices::cube_texture_pool_index,
              Render->CubeTexturesSystem->CubeTextureTableGPU);
            currentCommandList->SetGraphicsRootShaderResourceView(
              (int)root_parameters_draw_indices::bone_mapping_pool_index,
              Render->BoneMappingSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRootDescriptorTable(
              (int)root_parameters_draw_indices::oit_texture_index,
              Render->OITTransparencySystem->TextureUAVGPUDescriptor);
            currentCommandList->SetGraphicsRootDescriptorTable(
              (int)root_parameters_draw_indices::oit_pool_index,
              Render->OITTransparencySystem->NodesPoolUAVGPUDescriptor);
        }

        currentCommandList->DrawIndexedInstanced(command.DrawArguments.IndexCountPerInstance, 1, 0, 0, 0);
    }
}

void gdr::oit_color_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  if (!Render->Params.IsTransparent)
    return;

  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_frame_hdr);

  // set common params
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  currentCommandList->SetGraphicsRootConstantBufferView(
    (int)root_parameters_draw_indices::globals_buffer_index,
    Render->GlobalsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootConstantBufferView(
      (int)root_parameters_draw_indices::enviroment_buffer_index,
      Render->EnviromentSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
  // root_parameters_draw_indices::index_buffer_index will be set via indirect
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::object_transform_pool_index,
    Render->ObjectTransformsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::node_transform_pool_index,
    Render->NodeTransformsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::material_pool_index,
    Render->MaterialsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::lights_pool_index,
    Render->LightsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::texture_pool_index,
    Render->TexturesSystem->TextureTableGPU);
  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::cube_texture_pool_index,
    Render->CubeTexturesSystem->CubeTextureTableGPU);
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::bone_mapping_pool_index,
    Render->BoneMappingSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::oit_texture_index,
    Render->OITTransparencySystem->TextureUAVGPUDescriptor);
  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::oit_pool_index,
    Render->OITTransparencySystem->NodesPoolUAVGPUDescriptor);

  currentCommandList->ExecuteIndirect(
    CommandSignature,
    (UINT)Render->DrawCommandsSystem->AllocatedSize(),
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::TransparentsCulled].Resource,
    0,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::TransparentsCulled].Resource,
    Render->DrawCommandsSystem->CounterOffset); // stride to counter
}


gdr::oit_color_pass::~oit_color_pass(void)
{
    CommandSignature->Release();
    PSO->Release();
    RootSignature->Release();
    VertexShader->Release();
    PixelShader->Release();
}