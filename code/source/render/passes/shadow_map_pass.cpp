#include "p_header.h"

void gdr::shadow_map_pass::Initialize(void)
{
  // 1) Compile our shader
  Render->GetDevice().CompileShader(_T("bin/shaders/visibility/depth_draw.hlsl"), {}, shader_stage::Vertex, &VertexShader);

  // 2) Create root signature for draw depth
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);

    params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView(GDRGPUGlobalDataConstantBufferSlot);
    params[(int)root_parameters_draw_indices::index_buffer_index].InitAsConstants(sizeof(GDRGPUObjectIndices) / sizeof(int32_t), (int)GDRGPUObjectIndicesConstantBufferSlot);
    static_assert(GDRGPUObjectIndicesRecordRootIndex == (int)root_parameters_draw_indices::index_buffer_index, "Index buffer not in right root signature index");
    params[(int)root_parameters_draw_indices::object_transform_pool_index].InitAsShaderResourceView(GDRGPUObjectTransformPoolSlot);
    params[(int)root_parameters_draw_indices::node_transform_pool_index].InitAsShaderResourceView(GDRGPUNodeTransformPoolSlot);
    params[(int)root_parameters_draw_indices::bone_mapping_pool_index].InitAsShaderResourceView(GDRGPUBoneMappingSlot);

    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
    }
  }

  // 3) Create PSO for draw depth
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { defaultInputElementLayout, _countof(defaultInputElementLayout) };
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

  // 4) Create Command signature
  Render->GetDevice().GetDXDevice()->CreateCommandSignature(&Render->DrawCommandsSystem->commandSignatureDesc, RootSignature, IID_PPV_ARGS(&CommandSignature));
}

void gdr::shadow_map_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // saved globals
  GDRGPUGlobalData saved = Render->GlobalsSystem->Get();
  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_display);
  for (gdr_index i = 0; i < Render->LightsSystem->AllocatedSize(); i++)
  {
    if (Render->LightsSystem->IsExist(i) && Render->LightsSystem->Get(i).ShadowMapIndex != NONE_INDEX)
    {
      gdr_index ShadowMap = Render->LightsSystem->Get(i).ShadowMapIndex;
      gdr_index LightIndex = i;

      Render->GetDevice().TransitResourceState(currentCommandList, Render->ShadowMapsSystem->Get(ShadowMap).TextureResource.Resource,
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
      
      D3D12_CPU_DESCRIPTOR_HANDLE DSV = Render->RenderTargetsSystem->DepthStencilView;
      DSV.ptr += Render->GetDevice().GetDSVDescSize() * (ShadowMap + 1);

      // Scissor rect
      D3D12_RECT Rect;
      Rect.left = 0;
      Rect.top = 0;
      Rect.bottom = Render->ShadowMapsSystem->Get(ShadowMap).H;
      Rect.right = Render->ShadowMapsSystem->Get(ShadowMap).W;
      currentCommandList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 1, &Rect);

      D3D12_VIEWPORT Viewport;
      Viewport.TopLeftX = 0.0f;
      Viewport.TopLeftY = 0.0f;
      Viewport.Height = Rect.bottom - Rect.top;
      Viewport.Width = Rect.right - Rect.left;
      Viewport.MinDepth = 0.0f;
      Viewport.MaxDepth = 1.0f;

      Viewport.Height = max(Viewport.Height, 1);
      Viewport.Width = max(Viewport.Width, 1);

      currentCommandList->RSSetViewports(1, &Viewport);
      currentCommandList->RSSetScissorRects(1, &Rect);
      currentCommandList->OMSetRenderTargets(0, nullptr, false, &DSV);

      Render->GetDevice().SetCommandListAsUpload(currentCommandList);
      Render->GlobalsSystem->GetEditable().VP = Render->LightsSystem->Get(LightIndex).VP;
      mth::matr4f transform = Render->ObjectTransformsSystem->Get(Render->LightsSystem->Get(LightIndex).ObjectTransformIndex).Transform;
      Render->GlobalsSystem->GetEditable().CameraPos = { transform[3][0], transform[3][1], transform[3][2] };
      Render->GlobalsSystem->UpdateGPUData(currentCommandList);
      Render->GetDevice().ClearUploadListReference();

      // set common params
      currentCommandList->SetPipelineState(PSO);
      currentCommandList->SetGraphicsRootSignature(RootSignature);
      currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

      // just iterate for every draw call
      for (auto& i : Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::OpaqueAll])
      {
        if (!Render->DrawCommandsSystem->IsExist(i))
          continue;
        auto& command = Render->DrawCommandsSystem->Get(i);
        currentCommandList->IASetVertexBuffers(0, 1, &command.VertexBuffer);
        currentCommandList->IASetIndexBuffer(&command.IndexBuffer);
        {
          currentCommandList->SetGraphicsRootConstantBufferView(
            (int)root_parameters_draw_indices::globals_buffer_index,
            Render->GlobalsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
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
            (int)root_parameters_draw_indices::bone_mapping_pool_index,
            Render->BoneMappingSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
        }
        currentCommandList->DrawIndexedInstanced(command.DrawArguments.IndexCountPerInstance, 1, 0, 0, 0);
      }

      Render->GetDevice().TransitResourceState(currentCommandList, Render->ShadowMapsSystem->Get(ShadowMap).TextureResource.Resource,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
    }
  }

  Render->GetDevice().SetCommandListAsUpload(currentCommandList);
  Render->GlobalsSystem->GetEditable() = saved;
  Render->GlobalsSystem->UpdateGPUData(currentCommandList);
  Render->GetDevice().ClearUploadListReference();
  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_frame_hdr);
}

void gdr::shadow_map_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  GDRGPUGlobalData saved = Render->GlobalsSystem->Get();
  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_display);
  for (gdr_index i = 0; i < Render->LightsSystem->AllocatedSize(); i++)
  {
    if (Render->LightsSystem->IsExist(i) && Render->LightsSystem->Get(i).ShadowMapIndex != NONE_INDEX)
    {
      gdr_index ShadowMap = Render->LightsSystem->Get(i).ShadowMapIndex;
      gdr_index LightIndex = i;

      Render->GetDevice().TransitResourceState(currentCommandList, Render->ShadowMapsSystem->Get(ShadowMap).TextureResource.Resource,
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

      D3D12_CPU_DESCRIPTOR_HANDLE DSV = Render->RenderTargetsSystem->DepthStencilView;
      DSV.ptr += Render->GetDevice().GetDSVDescSize() * (ShadowMap + 1);

      // Scissor rect
      D3D12_RECT Rect;
      Rect.left = 0;
      Rect.top = 0;
      Rect.bottom = Render->ShadowMapsSystem->Get(ShadowMap).H;
      Rect.right = Render->ShadowMapsSystem->Get(ShadowMap).W;
      currentCommandList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 1, &Rect);

      Render->GetDevice().SetCommandListAsUpload(currentCommandList);
      Render->GlobalsSystem->GetEditable().VP = Render->LightsSystem->Get(LightIndex).VP;
      mth::matr4f transform = Render->ObjectTransformsSystem->Get(Render->LightsSystem->Get(LightIndex).ObjectTransformIndex).Transform;
      Render->GlobalsSystem->GetEditable().CameraPos = {transform[3][0], transform[3][1], transform[3][2]};
      Render->GlobalsSystem->UpdateGPUData(currentCommandList);
      Render->GetDevice().ClearUploadListReference();

      D3D12_VIEWPORT Viewport;
      Viewport.TopLeftX = 0.0f;
      Viewport.TopLeftY = 0.0f;
      Viewport.Height = Rect.bottom - Rect.top;
      Viewport.Width = Rect.right - Rect.left;
      Viewport.MinDepth = 0.0f;
      Viewport.MaxDepth = 1.0f;

      Viewport.Height = max(Viewport.Height, 1);
      Viewport.Width = max(Viewport.Width, 1);

      currentCommandList->RSSetViewports(1, &Viewport);
      currentCommandList->RSSetScissorRects(1, &Rect);
      currentCommandList->OMSetRenderTargets(0, nullptr, false, &DSV);


      // set common params
      currentCommandList->SetPipelineState(PSO);
      currentCommandList->SetGraphicsRootSignature(RootSignature);
      currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

      currentCommandList->SetGraphicsRootConstantBufferView(
        (int)root_parameters_draw_indices::globals_buffer_index,
        Render->GlobalsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
      // root_parameters_draw_indices::index_buffer_index will be set via indirect
      currentCommandList->SetGraphicsRootShaderResourceView(
        (int)root_parameters_draw_indices::object_transform_pool_index,
        Render->ObjectTransformsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRootShaderResourceView(
        (int)root_parameters_draw_indices::node_transform_pool_index,
        Render->NodeTransformsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRootShaderResourceView(
        (int)root_parameters_draw_indices::bone_mapping_pool_index,
        Render->BoneMappingSystem->GetGPUResource().Resource->GetGPUVirtualAddress());

      currentCommandList->ExecuteIndirect(
        CommandSignature,
        (UINT)Render->DrawCommandsSystem->AllocatedSize(),
        Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueAll].Resource,
        0,
        Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueAll].Resource,
        Render->DrawCommandsSystem->CounterOffset); // stride to counter

      Render->GetDevice().TransitResourceState(currentCommandList, Render->ShadowMapsSystem->Get(ShadowMap).TextureResource.Resource,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
    }
  }
  Render->GetDevice().SetCommandListAsUpload(currentCommandList);
  Render->GlobalsSystem->GetEditable() = saved;
  Render->GlobalsSystem->UpdateGPUData(currentCommandList);
  Render->GetDevice().ClearUploadListReference();
  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_frame_hdr);
}

gdr::shadow_map_pass::~shadow_map_pass(void)
{
  RootSignature->Release();
  PSO->Release();
  VertexShader->Release();
  CommandSignature->Release();
}
