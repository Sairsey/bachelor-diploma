#include "p_header.h"

void gdr::visibility_hier_depth_pass::Initialize(void)
{
  // 1) Compile our shader
  Render->GetDevice().CompileShader(_T("bin/shaders/visibility/depth_draw.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/shared/mip_generate.hlsl"), {}, shader_stage::Compute, &ComputeShader);

  // 2) Create root signature for draw depth
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);

    params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView(GDRGPUGlobalDataConstantBufferSlot);
    params[(int)root_parameters_draw_indices::index_buffer_index].InitAsConstants(sizeof(GDRGPUObjectIndices) / sizeof(int32_t), (int)GDRGPUObjectIndicesConstantBufferSlot);
    static_assert(GDRGPUObjectIndicesRecordRootIndex == (int)root_parameters_draw_indices::index_buffer_index, "Index buffer not in right root signature index");
    params[(int)root_parameters_draw_indices::object_transform_pool_index].InitAsShaderResourceView(GDRGPUObjectTransformPoolSlot);
    params[(int)root_parameters_draw_indices::node_transform_pool_index].InitAsShaderResourceView(GDRGPUNodeTransformPoolSlot);

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

  // 5) Create root signature for mip generation
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    CD3DX12_DESCRIPTOR_RANGE descr[2] = {};

    params.resize((int)root_parameters_compute_indices::total_root_parameters);

    params[(int)root_parameters_compute_indices::mips_params_index].InitAsConstants(4, GDRGPUUserConstantBuffer1Slot);

    {
      descr[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUUserUnorderedAccess1Slot);
      params[(int)root_parameters_compute_indices::input_srv_index].InitAsDescriptorTable(1, &descr[0]);
    }

    {
      descr[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUUserUnorderedAccess2Slot);
      params[(int)root_parameters_compute_indices::output_uav_index].InitAsDescriptorTable(1, &descr[1]);
    }

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &ComputeRootSignature);
  }

  // 6) Create root signature for mip generation
  {
    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = ComputeRootSignature;
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(ComputeShader);

    Render->GetDevice().CreateComputePSO(computePsoDesc, &ComputePSO);
  }

  for (int i = 0; i < MAX_AMOUNT_OF_MIPS; i++)
    Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptorHandles[i], GPUDescriptorHandles[i]);
}

void gdr::visibility_hier_depth_pass::Generate(ID3D12GraphicsCommandList* currentCommandList)
{
  PROFILE_BEGIN(currentCommandList, "Hier Depth Mips Gen");
  int MipsAmount = CalculateMipMapsAmount((UINT)Render->DepthBuffer.Resource->GetDesc().Width, (UINT)Render->DepthBuffer.Resource->GetDesc().Height);
  
  // copy depth to biggest mip of Hier Depth texture
  PROFILE_BEGIN(currentCommandList, "Copy depth to texture");
  {
    Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->DepthBuffer.Resource,
      D3D12_RESOURCE_STATE_DEPTH_WRITE,
      D3D12_RESOURCE_STATE_COPY_SOURCE);

    Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->RenderTargetsSystem->HierDepthTexture.Resource,
      D3D12_RESOURCE_STATE_COMMON,
      D3D12_RESOURCE_STATE_COPY_DEST);

    D3D12_TEXTURE_COPY_LOCATION Dest;
    Dest.pResource = Render->RenderTargetsSystem->HierDepthTexture.Resource;
    Dest.SubresourceIndex = 0;
    Dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

    D3D12_TEXTURE_COPY_LOCATION Source;
    Source.pResource = Render->DepthBuffer.Resource;
    Source.SubresourceIndex = 0;
    Source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

    currentCommandList->CopyTextureRegion(&Dest, 0, 0, 0, &Source, NULL);

    // Transit all resource states back
    Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->RenderTargetsSystem->HierDepthTexture.Resource,
      D3D12_RESOURCE_STATE_COPY_DEST,
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->DepthBuffer.Resource,
      D3D12_RESOURCE_STATE_COPY_SOURCE,
      D3D12_RESOURCE_STATE_DEPTH_WRITE);
  }
  PROFILE_END(currentCommandList);

  currentCommandList->SetPipelineState(ComputePSO);
  currentCommandList->SetComputeRootSignature(ComputeRootSignature);

  struct {
    UINT PrevW;
    UINT PrevH;
    UINT W;
    UINT H;
  } MipsParams = { (UINT)Render->DepthBuffer.Resource->GetDesc().Width, (UINT)Render->DepthBuffer.Resource->GetDesc().Height };

  MipsParams.W = MipsParams.PrevW / 2;
  MipsParams.H = MipsParams.PrevH / 2;

  // Generate UAV-s
  for (int i = 0; i < MipsAmount; i++)
  {
    // generate correct UAV Descriptor
    {
      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
      uavDesc.Format = Render->RenderTargetsSystem->HierDepthTexture.Resource->GetDesc().Format;
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
      uavDesc.Texture2D.MipSlice = i;
      uavDesc.Texture2D.PlaneSlice = 0;

      Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(
        Render->RenderTargetsSystem->HierDepthTexture.Resource,
        NULL,
        &uavDesc,
        CPUDescriptorHandles[i]);
    }
  }

  // Calculate Mips
  for (int i = 1; i < MipsAmount; i++)
  {
    currentCommandList->SetComputeRoot32BitConstants((int)root_parameters_compute_indices::mips_params_index, 4, &MipsParams, 0);
    currentCommandList->SetComputeRootDescriptorTable((int)root_parameters_compute_indices::input_srv_index, GPUDescriptorHandles[(i - 1)]);
    currentCommandList->SetComputeRootDescriptorTable((int)root_parameters_compute_indices::output_uav_index, GPUDescriptorHandles[i]);

    currentCommandList->Dispatch(max((UINT)ceil(MipsParams.W / 32.0), 1u), max((UINT)ceil(MipsParams.H / 32.0), 1u), 1);

    MipsParams.PrevW = MipsParams.W;
    MipsParams.PrevH = MipsParams.H;

    MipsParams.W = MipsParams.PrevW / 2;
    MipsParams.H = MipsParams.PrevH / 2;

    CD3DX12_RESOURCE_BARRIER bar = CD3DX12_RESOURCE_BARRIER::UAV(Render->RenderTargetsSystem->HierDepthTexture.Resource);
    currentCommandList->ResourceBarrier(1, &bar);
  }

  // Transit all resource states back
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->RenderTargetsSystem->HierDepthTexture.Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
    D3D12_RESOURCE_STATE_COMMON);
  PROFILE_END(currentCommandList);
}

void gdr::visibility_hier_depth_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  if (Render->Params.IsViewLocked)
    return; // Keep previous HierDepth

  // set common params
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // just iterate for every draw call
  for (auto& i : Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::OpaqueFrustrumCulled])
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
    }
    currentCommandList->DrawIndexedInstanced(command.DrawArguments.IndexCountPerInstance, 1, 0, 0, 0);
  }

  Generate(currentCommandList);
}

void gdr::visibility_hier_depth_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  if (Render->Params.IsViewLocked)
    return; // Keep previous HierDepth

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

  currentCommandList->ExecuteIndirect(
    CommandSignature,
    (UINT)Render->DrawCommandsSystem->AllocatedSize(),
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueFrustrumCulled].Resource,
    0,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueFrustrumCulled].Resource,
    Render->DrawCommandsSystem->CounterOffset); // stride to counter

  Generate(currentCommandList);
}

gdr::visibility_hier_depth_pass::~visibility_hier_depth_pass(void)
{
  ComputeRootSignature->Release();
  ComputePSO->Release();
  ComputeShader->Release();

  RootSignature->Release();
  PSO->Release();
  VertexShader->Release();
  CommandSignature->Release();
}
