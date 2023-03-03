#include "p_header.h"

void gdr::debug_hier_pass::Initialize(void)
{
    // 1) Compile our shaders
    Render->GetDevice().CompileShader(_T("bin/shaders/debug/debug_hier.hlsl"), {}, shader_stage::Vertex, &VertexShader);
    Render->GetDevice().CompileShader(_T("bin/shaders/debug/debug_hier.hlsl"), {}, shader_stage::Pixel, &PixelShader);

    // 2) Create root signature 
    {
        std::vector<CD3DX12_ROOT_PARAMETER> params;
        params.resize((int)root_parameters_draw_indices::total_root_parameters);

        params[(int)root_parameters_draw_indices::params_buffer_index].InitAsConstants(sizeof(ShaderParams) / sizeof(int32_t), GDRGPUUserConstantBuffer1Slot);
        params[(int)root_parameters_draw_indices::node_transforms_buffer_index].InitAsShaderResourceView(GDRGPUNodeTransformPoolSlot);

        {
            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init((UINT)params.size(), &params[0], (UINT)Render->TexturesSystem->SamplersDescs.size(), Render->TexturesSystem->SamplersDescs.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
        }
    }

    // 3) Create Input layout
    D3D12_INPUT_ELEMENT_DESC inputElementLayout[] =
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // 3) Create PSO
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementLayout, _countof(inputElementLayout) };
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
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = Render->RenderTargetsSystem->TargetParams[(int)render_targets_enum::target_display].Format;
        psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
        psoDesc.SampleDesc.Count = 1;

        Render->GetDevice().CreatePSO(psoDesc, &PSO);
    }

    // 4) Create vertex buffer and index buffer
    std::vector<mth::vec3f> Vertices;
    std::vector<UINT32> Indices;
    Vertices.push_back({ 0, 0, 0 }); // x y z 0
    Vertices.push_back({ 1, 1, 1 });  // x y Z 1
    
    Indices.push_back(0); Indices.push_back(1);
    
    IndexCount = (UINT)Indices.size();

    ID3D12GraphicsCommandList* commandList;
    Render->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "debug_hier_pass create line");
    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer(3 * sizeof(float) * Vertices.size()),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        nullptr,
        VertexBuffer,
        &Vertices[0],
        3 * sizeof(float) * Vertices.size());
      VertexBuffer.Resource->SetName(L"Hier line vertex buffer");
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
      IndexBuffer.Resource->SetName(L"Hier line index buffer");
    }
    {
      IndexBufferView.BufferLocation = IndexBuffer.Resource->GetGPUVirtualAddress();
      IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
      IndexBufferView.SizeInBytes = (UINT)(sizeof(UINT32) * Indices.size());
    }

    PROFILE_END(commandList);
    Render->GetDevice().CloseUploadCommandList();
}

void gdr::debug_hier_pass::DrawHier(ID3D12GraphicsCommandList* currentCommandList, gdr_index Me, gdr_index ObjectTransformIndex)
{
  if (!Render->NodeTransformsSystem->IsExist(Me))
    return;

  if (Render->NodeTransformsSystem->Get(Me).ParentIndex != NONE_INDEX)
  {
    ShaderParams params;
    params.VP = Render->GlobalsSystem->Get().VP;
    params.Element = Me;
    params.RootTransform = 
      Render->ObjectTransformsSystem->IsExist(ObjectTransformIndex) ? 
      Render->ObjectTransformsSystem->Get(ObjectTransformIndex).Transform :
      mth::matr4f::Identity();

    currentCommandList->SetGraphicsRoot32BitConstants(
      (int)root_parameters_draw_indices::params_buffer_index,
      sizeof(ShaderParams) / sizeof(int32_t),
      &params, 0);
    currentCommandList->SetGraphicsRootShaderResourceView(
      (int)root_parameters_draw_indices::node_transforms_buffer_index,
      Render->NodeTransformsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());

    currentCommandList->DrawIndexedInstanced(IndexCount, 1, 0, 0, 0);
  }

  gdr_index IndexToDraw = Render->NodeTransformsSystem->Get(Me).ChildIndex;
  while (IndexToDraw != NONE_INDEX)
  {
    DrawHier(currentCommandList, IndexToDraw, ObjectTransformIndex);
    IndexToDraw = Render->NodeTransformsSystem->Get(IndexToDraw).NextIndex;
  }
}

void gdr::debug_hier_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
    if (!Render->Params.IsShowHier)
      return;

    Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_display);

    // set common params
    currentCommandList->SetPipelineState(PSO);
    currentCommandList->SetGraphicsRootSignature(RootSignature);
    currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    currentCommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
    currentCommandList->IASetIndexBuffer(&IndexBufferView);

    // just iterate for every draw call
    for (int i = 0; i < Render->DrawCommandsSystem->AllocatedSize(); i++)
    {
        if (!Render->DrawCommandsSystem->IsExist(i))
          continue;

        // get bone Mapping system
        if (Render->DrawCommandsSystem->Get(i).Indices.BoneMappingIndex != NONE_INDEX)
        {
          gdr_index boneMapping = Render->DrawCommandsSystem->Get(i).Indices.BoneMappingIndex;
          gdr_index rootNode = Render->BoneMappingSystem->Get(boneMapping).BoneMapping[0];

          while (rootNode != NONE_INDEX && Render->NodeTransformsSystem->Get(rootNode).ParentIndex != NONE_INDEX)
            rootNode = Render->NodeTransformsSystem->Get(rootNode).ParentIndex;
          
          DrawHier(currentCommandList, rootNode, Render->DrawCommandsSystem->Get(i).Indices.ObjectTransformIndex);
        }
    }
}

gdr::debug_hier_pass::~debug_hier_pass(void)
{
    Render->GetDevice().ReleaseGPUResource(VertexBuffer);
    Render->GetDevice().ReleaseGPUResource(IndexBuffer);
    PSO->Release();
    RootSignature->Release();
    VertexShader->Release();
    PixelShader->Release();
}