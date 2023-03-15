#include "p_header.h"

void gdr::shadow_map_pass::Initialize(void)
{
  // 1) Compile our shader
  Render->GetDevice().CompileShader(_T("bin/shaders/visibility/depth_draw.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/visibility/shadow_map_frustum.hlsl"), {}, shader_stage::Compute, &ComputeShader);

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
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; // due to opengl matrices, CULL_MODE_FRONT and CULL_MODE_BACK are swapped
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

  // 5) Create root signature for calculate frustum depth
  {
      std::vector<CD3DX12_ROOT_PARAMETER> params;
      params.resize((int)root_parameters_compute_indices::total_root_parameters);

      params[(int)root_parameters_compute_indices::compute_globals_index].InitAsConstants(sizeof(GDRGPUComputeGlobals) / sizeof(int32_t), GDRGPUComputeGlobalDataConstantBufferSlot);
      params[(int)root_parameters_compute_indices::object_transform_pool_index].InitAsShaderResourceView(GDRGPUObjectTransformPoolSlot);
      params[(int)root_parameters_compute_indices::all_commands_pool_index].InitAsShaderResourceView(GDRGPUAllCommandsPoolSlot);

      CD3DX12_DESCRIPTOR_RANGE descr[1] = {};

      {
          descr[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUShadowMapCommandsPoolSlot);
          params[(int)root_parameters_compute_indices::shadow_map_commands_pool_index].InitAsDescriptorTable(1, &descr[0]);
      }

      {
          CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
          rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
          Render->GetDevice().CreateRootSignature(rootSignatureDesc, &ComputeRootSignature);
      }
  }

  // 3) Create PSO for calculate frustum depth
  {
      // Describe and create the compute pipeline state object (PSO).
      D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
      computePsoDesc.pRootSignature = ComputeRootSignature;
      computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(ComputeShader);

      Render->GetDevice().CreateComputePSO(computePsoDesc, &ComputePSO);
  }
}

static bool CullAABBFrustum(
    float4x4 VP,
    float4x4 transform,
    float3 minAABB,
    float3 maxAABB)
{
    // Use our min max to define eight corners
    float3 corners[8] = {
        float3(minAABB.X, minAABB.Y, minAABB.Z), // x y z
        float3(maxAABB.X, minAABB.Y, minAABB.Z), // X y z
        float3(minAABB.X, maxAABB.Y, minAABB.Z), // x Y z
        float3(maxAABB.X, maxAABB.Y, minAABB.Z), // X Y z

        float3(minAABB.X, minAABB.Y, maxAABB.Z), // x y Z
        float3(maxAABB.X, minAABB.Y, maxAABB.Z), // X y Z
        float3(minAABB.X, maxAABB.Y, maxAABB.Z), // x Y Z
        float3(maxAABB.X, maxAABB.Y, maxAABB.Z) // X Y Z
    };

    float4x4 matr = transform * VP;
    for (int corner_idx = 0; corner_idx < 8; corner_idx++)
    {
        if (corners[corner_idx].X * matr[0][3] + corners[corner_idx].Y * matr[1][3] + corners[corner_idx].Z * matr[2][3] + matr[3][3] > 0)
            corners[corner_idx] = corners[corner_idx] * matr;
        else if (corners[corner_idx].X * matr[0][3] + corners[corner_idx].Y * matr[1][3] + corners[corner_idx].Z * matr[2][3] + matr[3][3] == 0)
            corners[corner_idx] = {0, 0, 0};
        else
            corners[corner_idx] = corners[corner_idx] * matr * -1;
    }

    bool LeftPlaneResult = true;
    bool RightPlaneResult = true;
    bool TopPlaneResult = true;
    bool BottomPlaneResult = true;
    bool FarPlaneResult = true;
    bool NearPlaneResult = true;

    for (int corner_idx = 0; corner_idx < 8; corner_idx++)
    {
        LeftPlaneResult = LeftPlaneResult && (corners[corner_idx].X < -1);
        RightPlaneResult = RightPlaneResult && (corners[corner_idx].X > 1);

        BottomPlaneResult = BottomPlaneResult && (corners[corner_idx].Y <= -1);
        TopPlaneResult = TopPlaneResult && (corners[corner_idx].Y >= 1);

        FarPlaneResult = FarPlaneResult && (corners[corner_idx].Z >= 1);
        NearPlaneResult = NearPlaneResult && (corners[corner_idx].Z <= 0);
    }

    bool inside = !(LeftPlaneResult || RightPlaneResult || TopPlaneResult || BottomPlaneResult || FarPlaneResult || NearPlaneResult);
    return inside;
}

void gdr::shadow_map_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // saved globals
  GDRGPUGlobalData saved = Render->GlobalsSystem->Get();
  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_frame_final);
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
      for (auto& i : Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::All])
      {
        if (!Render->DrawCommandsSystem->IsExist(i))
          continue;
        auto& command = Render->DrawCommandsSystem->Get(i);

        bool visible = CullAABBFrustum(
                Render->GlobalsSystem->Get().VP,
                Render->ObjectTransformsSystem->Get(command.Indices.ObjectTransformIndex).Transform,
                Render->ObjectTransformsSystem->Get(command.Indices.ObjectTransformIndex).minAABB,
                Render->ObjectTransformsSystem->Get(command.Indices.ObjectTransformIndex).maxAABB);
        bool transparent = command.Indices.ObjectParamsMask & OBJECT_PARAMETER_TRANSPARENT;

        if (!visible || transparent)
            continue;

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

  // Transit resource state of valid buffers to INDIRECT_ARGUMENT.
  Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::Shadow].Resource,
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

void gdr::shadow_map_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // Transit resource state of valid buffers to INDIRECT_ARGUMENT.
  Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::Shadow].Resource,
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

  GDRGPUGlobalData saved = Render->GlobalsSystem->Get();
  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_frame_tonemap);
  for (gdr_index i = 0; i < Render->LightsSystem->AllocatedSize(); i++)
  {
    if (Render->LightsSystem->IsExist(i) && Render->LightsSystem->Get(i).ShadowMapIndex != NONE_INDEX)
    {      
      gdr_index ShadowMap = Render->LightsSystem->Get(i).ShadowMapIndex;
      gdr_index LightIndex = i;

      PROFILE_BEGIN(currentCommandList, (std::string("Light #") + std::to_string(LightIndex) + std::string(" Shadow #") + std::to_string(ShadowMap)).c_str());

      PROFILE_BEGIN(currentCommandList, "Free UAV");
      /*
       * Free UAV
       */
      {
        Render->GetDevice().TransitResourceState(
          currentCommandList,
          Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::Shadow].Resource,
          D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_COPY_DEST);

        currentCommandList->CopyBufferRegion(
            Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::Shadow].Resource,
            Render->DrawCommandsSystem->CounterOffset,
            Render->DrawCommandsSystem->CommandsUAVReset.Resource,
            0,
            sizeof(UINT));

        Render->GetDevice().TransitResourceState(
            currentCommandList,
            Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::Shadow].Resource,
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
      }
      PROFILE_END(currentCommandList);

      /*
       * Calculate Frustum
       */
      PROFILE_BEGIN(currentCommandList, "Frustum cull");
      {
          currentCommandList->SetPipelineState(ComputePSO);

          ID3D12DescriptorHeap* pDescriptorHeaps = Render->GetDevice().GetDescriptorHeap();
          currentCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);
          currentCommandList->SetComputeRootSignature(ComputeRootSignature);

          GDRGPUComputeGlobals ComputeGlobals;

          // update ComputeGlobals
          {
              ComputeGlobals.VP = Render->LightsSystem->Get(LightIndex).VP;
              ComputeGlobals.width = Render->ShadowMapsSystem->Get(ShadowMap).W;
              ComputeGlobals.height = Render->ShadowMapsSystem->Get(ShadowMap).H;
              ComputeGlobals.frustumCulling = true;
              ComputeGlobals.occlusionCulling = false;
              ComputeGlobals.commandsCount = (UINT)Render->DrawCommandsSystem->AllocatedSize();
          }

          currentCommandList->SetComputeRoot32BitConstants(
              (int)root_parameters_compute_indices::compute_globals_index, // root parameter index
              sizeof(GDRGPUComputeGlobals) / sizeof(int32_t),
              &ComputeGlobals,
              0);
          currentCommandList->SetComputeRootShaderResourceView(
              (int)root_parameters_compute_indices::object_transform_pool_index,
              Render->ObjectTransformsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
          currentCommandList->SetComputeRootShaderResourceView(
              (int)root_parameters_compute_indices::all_commands_pool_index,
              Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::All].Resource->GetGPUVirtualAddress());
          currentCommandList->SetComputeRootDescriptorTable(
              (int)root_parameters_compute_indices::shadow_map_commands_pool_index,
              Render->DrawCommandsSystem->CommandsGPUDescriptor[(int)indirect_command_pools_enum::Shadow]);

          currentCommandList->Dispatch(static_cast<UINT>(ceil(ComputeGlobals.commandsCount / float(GDRGPUComputeThreadBlockSize))), 1, 1);

          Render->GetDevice().TransitResourceState(
              currentCommandList,
              Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::Shadow].Resource,
              D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
      }
      PROFILE_END(currentCommandList);

      /*
       * Use Frustum for draw
       */
      PROFILE_BEGIN(currentCommandList, "depth draw");
      {
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
          Render->GlobalsSystem->GetEditable().CameraPos = { transform[3][0], transform[3][1], transform[3][2] };
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
              Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::Shadow].Resource,
              0,
              Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::Shadow].Resource,
              Render->DrawCommandsSystem->CounterOffset); // stride to counter

          Render->GetDevice().TransitResourceState(currentCommandList, Render->ShadowMapsSystem->Get(ShadowMap).TextureResource.Resource,
              D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
      }
      PROFILE_END(currentCommandList);
      PROFILE_END(currentCommandList);
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
  ComputeRootSignature->Release();
  ComputePSO->Release();
  ComputeShader->Release();

  RootSignature->Release();
  PSO->Release();
  VertexShader->Release();
  CommandSignature->Release();
}
