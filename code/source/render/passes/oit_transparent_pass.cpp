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

void gdr::oit_transparent_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/OITTransparentColor.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/OITTransparentColor.hlsl"), {}, shader_stage::Pixel, &PixelShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/OITTransparentCompose.hlsl"), {}, shader_stage::Vertex, &ComposeVertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/OITTransparentCompose.hlsl"), {}, shader_stage::Pixel, &ComposePixelShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/OITTransparentCull.hlsl"), {}, shader_stage::Compute, &ComputeShader);

  // 2) Create root signature 
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);
    CD3DX12_DESCRIPTOR_RANGE bindlessTexturesDesc[1];  // Textures Pool
    CD3DX12_DESCRIPTOR_RANGE cubeBindlessTexturesDesc[1];  // Textures Pool
    CD3DX12_DESCRIPTOR_RANGE descr = {};

    params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView((int)transparent_buffer_registers::globals_buffer_register);
    
    params[(int)root_parameters_draw_indices::index_buffer_index].InitAsConstants(sizeof(ObjectIndices) / sizeof(int32_t), (int)transparent_buffer_registers::index_buffer_register);
    params[(int)root_parameters_draw_indices::transform_pool_index].InitAsShaderResourceView((int)transparent_texture_registers::object_transform_pool_register);
    params[(int)root_parameters_draw_indices::material_pool_index].InitAsShaderResourceView((int)transparent_texture_registers::material_pool_register);
    params[(int)root_parameters_draw_indices::light_sources_pool_index].InitAsShaderResourceView((int)transparent_texture_registers::light_sources_pool_register);

    {
      bindlessTexturesDesc[0].BaseShaderRegister = (int)transparent_texture_registers::texture_pool_register;
      bindlessTexturesDesc[0].NumDescriptors = MAX_TEXTURE_AMOUNT;
      bindlessTexturesDesc[0].OffsetInDescriptorsFromTableStart = 0;
      bindlessTexturesDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      bindlessTexturesDesc[0].RegisterSpace = 1;

      params[(int)root_parameters_draw_indices::texture_pool_index].InitAsDescriptorTable(1, bindlessTexturesDesc);
    }

    {
      cubeBindlessTexturesDesc[0].BaseShaderRegister = (int)transparent_texture_registers::cube_texture_pool_register;
      cubeBindlessTexturesDesc[0].NumDescriptors = MAX_CUBE_TEXTURE_AMOUNT;
      cubeBindlessTexturesDesc[0].OffsetInDescriptorsFromTableStart = 0;
      cubeBindlessTexturesDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      cubeBindlessTexturesDesc[0].RegisterSpace = 2;

      params[(int)root_parameters_draw_indices::cube_texture_pool_index].InitAsDescriptorTable(1, cubeBindlessTexturesDesc);
    }

    params[(int)root_parameters_draw_indices::oit_lists_index].InitAsUnorderedAccessView((int)transparent_uav_registers::oit_lists_register);

    // UAV set as Descriptor range
    {
      descr.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, (int)transparent_uav_registers::oit_pool_register);
      params[(int)root_parameters_draw_indices::oit_pool_index].InitAsDescriptorTable(1, &descr);
    }


    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[2];
    samplerDescs[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR); // Texture sampler
    samplerDescs[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_POINT); // Texture sampler

    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init((UINT)params.size(), &params[0], sizeof(samplerDescs) / sizeof(samplerDescs[0]), samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
    }
  }

  // 3) Create InputLayout
  static const D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };

  // 4) Create PSO
  // Describe and create the graphics pipeline state object (PSO).
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, 4 };
    psoDesc.pRootSignature = RootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
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
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Render->RenderTargets->TargetParams[(int)render_targets_enum::target_frame_hdr].Format;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &PSO);
  }

  // 5) Create Buffers for OIT
  {
    UINT UAVSize = AlignForUavCounter(sizeof(OITNode) * MAX_AMOUNT_OF_TRANSPARENT_PIXELS);
    CounterOffset = UAVSize;

    // UAV for OIT Pool
    Render->GetDevice().AllocateStaticDescriptors(2, OITPoolCPUDescriptor, OITPoolGPUDescriptor);
    Render->GetDevice().CreateGPUResource(
      CD3DX12_RESOURCE_DESC::Buffer({ UAVSize + sizeof(UINT) }, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      OITPool);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = (UINT)MAX_AMOUNT_OF_TRANSPARENT_PIXELS;
    uavDesc.Buffer.StructureByteStride = sizeof(OITNode);
    uavDesc.Buffer.CounterOffsetInBytes = UAVSize;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(OITPool.Resource, OITPool.Resource, &uavDesc, OITPoolCPUDescriptor);
    OITPool.Resource->SetName(L"OIT Pool");

    // SRV for OIT Pool
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = (UINT)MAX_AMOUNT_OF_TRANSPARENT_PIXELS;
    srvDesc.Buffer.StructureByteStride = sizeof(OITNode);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = OITPoolCPUDescriptor;
    srvHandle.ptr += Render->GetDevice().GetSRVDescSize();
    Render->GetDevice().GetDXDevice()->CreateShaderResourceView(OITPool.Resource, &srvDesc, srvHandle);

    // UAV for OIT Lists
    Render->GetDevice().AllocateStaticDescriptors(2, OITListsCPUDescriptor, OITListsGPUDescriptor);
  }

  // 6) Create Combine InputLayout
  static const D3D12_INPUT_ELEMENT_DESC combineInputElementDescs[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };

  // 7) Create root signature 
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_compose_indices::total_root_parameters);

    params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView((int)transparent_buffer_registers::globals_buffer_register);

    params[(int)root_parameters_compose_indices::oit_lists_index].InitAsShaderResourceView((int)transparent_texture_registers::oit_lists_compose_register);
    params[(int)root_parameters_compose_indices::oit_pool_index].InitAsShaderResourceView((int)transparent_texture_registers::oit_pool_compose_register);

    if (params.size() != 0)
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &ComposeRootSignature);
    }
    else
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &ComposeRootSignature);
    }
  }

  // 8) Create Compose PSO
  // Describe and create the graphics pipeline state object (PSO).
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { combineInputElementDescs, 1 };
    psoDesc.pRootSignature = ComposeRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(ComposeVertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ComposePixelShader);
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
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Render->RenderTargets->TargetParams[(int)render_targets_enum::target_frame_hdr].Format;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &ComposePSO);
  }

  // 9) Create Vertices for fullscreen draw
  {
    mth::vec3f vertices[3] = {
    mth::vec3f(-1, -1, 0),
    mth::vec3f(-1, 3, 0),
    mth::vec3f(3, -1, 0)
  };

    ID3D12GraphicsCommandList* commandList;
    Render->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "oit_transparent pass create screen mesh");

    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer(3 * sizeof(float) * 3),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        nullptr,
        ScreenVertexBuffer,
        &vertices[0],
        3 * sizeof(float) * 3);
    }
    {
      ScreenVertexBufferView.BufferLocation = ScreenVertexBuffer.Resource->GetGPUVirtualAddress();
      ScreenVertexBufferView.StrideInBytes = (UINT)(3 * sizeof(float));
      ScreenVertexBufferView.SizeInBytes = (UINT)(3 * sizeof(float) * 3);
    }

    PROFILE_END(commandList);
    Render->GetDevice().CloseUploadCommandList();
  }

  // 10) Create root signature for compute
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    CD3DX12_DESCRIPTOR_RANGE descr = {};

    params.resize((int)root_parameters_compute_indices::total_root_parameters);

    {
      params[(int)root_parameters_compute_indices::compute_params_index].InitAsConstants(
        sizeof(ComputeRootConstants) / sizeof(int32_t),
        (int)transparent_buffer_registers::globals_buffer_register);
    }

    {
      params[(int)root_parameters_compute_indices::transform_pool_index].InitAsShaderResourceView(
        (int)transparent_texture_registers::object_transform_pool_register);
    }

    {
      params[(int)root_parameters_compute_indices::in_commands_pool_index].InitAsShaderResourceView(
        (int)transparent_texture_registers::command_pool_register);
    }

    // UAV set as Descriptor range
    {
      descr.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, (int)transparent_uav_registers::indirect_command_pool_register);

      params[(int)root_parameters_compute_indices::out_commands_pool_index].InitAsDescriptorTable(
        1, &descr);
    }

    if (params.size() != 0)
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init((UINT)params.size(), &params[0], 0U, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &ComputeRootSignature);
    }
    else
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init(0U, nullptr, 0U, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

      Render->GetDevice().CreateRootSignature(rootSignatureDesc, &ComputeRootSignature);
    }
  }

  // 11) Create Command signature
  {
    // Each command consists of a CBV update and a DrawInstanced call.
    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[4] = {};

    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = (int)root_parameters_draw_indices::index_buffer_index; // because Root parameter 0 is descriptor handle
    argumentDescs[0].Constant.Num32BitValuesToSet = sizeof(ObjectIndices) / sizeof(int32_t);

    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
    argumentDescs[1].VertexBuffer.Slot = 0;

    argumentDescs[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;

    argumentDescs[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
    commandSignatureDesc.pArgumentDescs = argumentDescs;
    commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
    commandSignatureDesc.ByteStride = sizeof(indirect_command);

    Render->GetDevice().GetDXDevice()->CreateCommandSignature(&commandSignatureDesc, RootSignature, IID_PPV_ARGS(&CommandSignature));
  }

  // 12) Create compute PSO
  {
    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = ComputeRootSignature;
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(ComputeShader);

    Render->GetDevice().CreateComputePSO(computePsoDesc, &ComputePSO);
  }
}

void gdr::oit_transparent_pass::CreateOITLists(void)
{
  CurrentOITListsSize = Render->GetEngine()->Width * Render->GetEngine()->Height * sizeof(OITList);
  Render->GetDevice().CreateGPUResource(
    CD3DX12_RESOURCE_DESC::Buffer({ CurrentOITListsSize }, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    OITLists);

  D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
  uavDesc.Format = DXGI_FORMAT_UNKNOWN;
  uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
  uavDesc.Buffer.FirstElement = 0;
  uavDesc.Buffer.NumElements = CurrentOITListsSize / sizeof(OITList);
  uavDesc.Buffer.StructureByteStride = sizeof(OITList);
  uavDesc.Buffer.CounterOffsetInBytes = 0;
  uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

  Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(OITLists.Resource, nullptr, &uavDesc, OITListsCPUDescriptor);
  OITLists.Resource->SetName(L"OIT Lists");

  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = DXGI_FORMAT_UNKNOWN;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Buffer.NumElements = CurrentOITListsSize / sizeof(OITList);
  srvDesc.Buffer.StructureByteStride = sizeof(OITList);
  srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

  D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = OITListsCPUDescriptor;
  srvHandle.ptr += Render->GetDevice().GetSRVDescSize();
  Render->GetDevice().GetDXDevice()->CreateShaderResourceView(OITLists.Resource, &srvDesc, srvHandle);

  OITListsClearVector.resize(CurrentOITListsSize / sizeof(OITList));
  for (int i = 0; i < OITListsClearVector.size(); i++)
  {
    OITListsClearVector[i].RootIndex = 0xFFFFFFFF;
  }
  OITListsClearBuffer.Resource = nullptr;
}

void gdr::oit_transparent_pass::CallCompute(ID3D12GraphicsCommandList* currentCommandList)
{
  if (!Render->Params.IsTransparent)
    return;

  // at first get our UAV
  OurUAVIndex = Render->IndirectSystem->CurrentUAV;
  Render->IndirectSystem->CurrentUAV = (Render->IndirectSystem->CurrentUAV + 1) % Render->IndirectSystem->TotalUAV;

  currentCommandList->CopyBufferRegion(
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    Render->IndirectSystem->CounterOffset,
    Render->IndirectSystem->CommandsUAVReset.Resource,
    0,
    sizeof(UINT));

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

  currentCommandList->SetPipelineState(ComputePSO);

  ID3D12DescriptorHeap* pDescriptorHeaps = Render->GetDevice().GetDescriptorHeap();
  currentCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);
  currentCommandList->SetComputeRootSignature(ComputeRootSignature);

  // update CPUComputeRootConstants
  CPUComputeRootConstants.VP = Render->PlayerCamera.GetVP();
  CPUComputeRootConstants.enableCulling = Render->Params.IsCulling;
  CPUComputeRootConstants.commandCount = (float)Render->IndirectSystem->CPUData.size();

  // set Root constants buffers etc
  currentCommandList->SetComputeRoot32BitConstants(
    (int)root_parameters_compute_indices::compute_params_index, // root parameter index
    sizeof(ComputeRootConstants) / sizeof(int32_t),
    &CPUComputeRootConstants,
    0);

  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_compute_indices::transform_pool_index, // root parameter index
    Render->TransformsSystem->GPUData.Resource->GetGPUVirtualAddress());

  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_compute_indices::in_commands_pool_index, // root parameter index
    Render->IndirectSystem->CommandsSRV.Resource->GetGPUVirtualAddress());

  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_compute_indices::out_commands_pool_index, // root parameter index
    Render->IndirectSystem->CommandsUAVGPUDescriptor[OurUAVIndex]);

  currentCommandList->Dispatch(static_cast<UINT>(ceil(Render->IndirectSystem->CPUData.size() / float(ComputeThreadBlockSize))), 1, 1);
}

void gdr::oit_transparent_pass::SyncCompute(ID3D12GraphicsCommandList* currentCommandList)
{
  if (!Render->Params.IsTransparent)
    return;

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

void gdr::oit_transparent_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_frame_hdr);
  if (!Render->Params.IsTransparent)
    return;
  // Update Globals
  Render->GlobalsSystem->CPUData.CameraPos = Render->PlayerCamera.GetPos();
  Render->GlobalsSystem->CPUData.VP = Render->PlayerCamera.GetVP();

  PROFILE_BEGIN(currentCommandList, "Update globals");
  Render->GetDevice().SetCommandListAsUpload(currentCommandList);
  Render->GlobalsSystem->UpdateGPUData(currentCommandList);
  Render->GetDevice().ClearUploadListReference();
  PROFILE_END(currentCommandList);

  // Clear and Update OIT
  {
    // Clear OIT Pool
    {
      currentCommandList->CopyBufferRegion(
        OITPool.Resource,
        CounterOffset,
        Render->IndirectSystem->CommandsUAVReset.Resource, // get buffer with 0 inside from indirect support
        0,
        sizeof(UINT));

      Render->GetDevice().TransitResourceState(
        currentCommandList,
        OITPool.Resource,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
    // Clear OIT List
    {
      // in case frame size change - delete and recreate
      if (Render->GetEngine()->Width * Render->GetEngine()->Height * sizeof(OITList) != 0 && 
        CurrentOITListsSize != Render->GetEngine()->Width * Render->GetEngine()->Height * sizeof(OITList))
      {
        Render->GetDevice().ReleaseGPUResource(OITLists);
        Render->GetDevice().ReleaseGPUResource(OITListsClearBuffer);
        CreateOITLists();
        if (OITListsClearBuffer.Resource == nullptr)
        {
          Render->GetDevice().SetCommandListAsUpload(currentCommandList);
          Render->GetDevice().CreateGPUResource(
            CD3DX12_RESOURCE_DESC::Buffer({ CurrentOITListsSize }),
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            OITListsClearBuffer, OITListsClearVector.data(), CurrentOITListsSize);
          Render->GetDevice().ClearUploadListReference();

        }
      }

      // Clear with 0xFFFFFFFF
      currentCommandList->CopyBufferRegion(
        OITLists.Resource,
        0,
        OITListsClearBuffer.Resource,
        0,
        CurrentOITListsSize);

      //Render->GetDevice().SetCommandListAsUpload(currentCommandList);
      //Render->GetDevice().UpdateBuffer(currentCommandList, OITLists.Resource, OITListsClearVector.data(), CurrentOITListsSize);
      //Render->GetDevice().ClearUploadListReference();



      Render->GetDevice().TransitResourceState(
        currentCommandList,
        OITLists.Resource,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
  }

  // set common params
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  std::vector<gdr::gdr_index> objects_to_draw;

  // gather all transparents
  for (gdr::gdr_index i = 0; i < Render->ObjectSystem->CPUPool.size(); i++)
  {
    if (Render->ObjectSystem->CPUPool[i].ObjectParams & OBJECT_PARAMETER_TRANSPARENT)
    {
      objects_to_draw.push_back(i);
    }
  }

  // draw them
  for (auto& i : objects_to_draw)
  {
    geometry& geom = Render->GeometrySystem->CPUPool[i];

    currentCommandList->IASetVertexBuffers(0, 1, &geom.VertexBufferView);
    currentCommandList->IASetIndexBuffer(&geom.IndexBufferView);
    {
      currentCommandList->SetGraphicsRootConstantBufferView(
        (int)root_parameters_draw_indices::globals_buffer_index,
        Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRoot32BitConstants(
        (int)root_parameters_draw_indices::index_buffer_index,
        sizeof(ObjectIndices) / sizeof(int32_t),
        &Render->ObjectSystem->CPUPool[i], 0);
      currentCommandList->SetGraphicsRootShaderResourceView(
        (int)root_parameters_draw_indices::transform_pool_index,
        Render->TransformsSystem->GPUData.Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRootShaderResourceView(
        (int)root_parameters_draw_indices::material_pool_index,
        Render->MaterialsSystem->GPUData.Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRootDescriptorTable(
        (int)root_parameters_draw_indices::texture_pool_index,
        Render->TexturesSystem->TextureTableGPU);
      currentCommandList->SetGraphicsRootUnorderedAccessView(
        (int)root_parameters_draw_indices::oit_lists_index,
        OITLists.Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRootShaderResourceView(
        (int)root_parameters_draw_indices::light_sources_pool_index,
        Render->LightsSystem->GPUData.Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRootDescriptorTable(
        (int)root_parameters_draw_indices::oit_pool_index,
        OITPoolGPUDescriptor);
      currentCommandList->SetGraphicsRootDescriptorTable(
        (int)root_parameters_draw_indices::cube_texture_pool_index,
        Render->CubeTexturesSystem->CubeTextureTableGPU);
    }

    currentCommandList->DrawIndexedInstanced(geom.IndexCount, 1, 0, 0, 0);
  }

  // after we drawn every object, we need to Draw fullscreen  with special shader
  currentCommandList->SetPipelineState(ComposePSO);
  currentCommandList->SetGraphicsRootSignature(ComposeRootSignature);
  currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

  currentCommandList->SetGraphicsRootConstantBufferView(
    (int)root_parameters_compose_indices::globals_buffer_index,
    Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_compose_indices::oit_lists_index,
    OITLists.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_compose_indices::oit_pool_index,
    OITPool.Resource->GetGPUVirtualAddress());

  currentCommandList->DrawInstanced(3, 1, 0, 0);

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    OITLists.Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    OITPool.Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
}

void gdr::oit_transparent_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  if (!Render->Params.IsTransparent)
    return;
  Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_frame_hdr);

  // Update Globals
  Render->GlobalsSystem->CPUData.CameraPos = Render->PlayerCamera.GetPos();
  Render->GlobalsSystem->CPUData.VP = Render->PlayerCamera.GetVP();

  PROFILE_BEGIN(currentCommandList, "Update globals");
  Render->GetDevice().SetCommandListAsUpload(currentCommandList);
  Render->GlobalsSystem->UpdateGPUData(currentCommandList);
  Render->GetDevice().ClearUploadListReference();
  PROFILE_END(currentCommandList);

  PROFILE_BEGIN(currentCommandList, "Clear OIT");
  // Clear and Update OIT
  {
    PROFILE_BEGIN(currentCommandList, "Clear Pool");
    // Clear OIT Pool
    {
      currentCommandList->CopyBufferRegion(
        OITPool.Resource,
        CounterOffset,
        Render->IndirectSystem->CommandsUAVReset.Resource, // get buffer with 0 inside from indirect support
        0,
        sizeof(UINT));

      Render->GetDevice().TransitResourceState(
        currentCommandList,
        OITPool.Resource,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
    PROFILE_END(currentCommandList);
    PROFILE_BEGIN(currentCommandList, "Clear List");
    // Clear OIT List
    {
      // in case frame size change - delete and recreate
      if (Render->GetEngine()->Width * Render->GetEngine()->Height * sizeof(OITList) != 0 &&
        CurrentOITListsSize != Render->GetEngine()->Width * Render->GetEngine()->Height * sizeof(OITList))
      {
        Render->GetDevice().ReleaseGPUResource(OITLists);
        Render->GetDevice().ReleaseGPUResource(OITListsClearBuffer);
        CreateOITLists();
        if (OITListsClearBuffer.Resource == nullptr)
        {
          Render->GetDevice().SetCommandListAsUpload(currentCommandList);
          Render->GetDevice().CreateGPUResource(
            CD3DX12_RESOURCE_DESC::Buffer({ CurrentOITListsSize }),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            OITListsClearBuffer, OITListsClearVector.data(), CurrentOITListsSize);
          OITListsClearBuffer.Resource->SetName(L"OIT Lists clear buffer");
          Render->GetDevice().ClearUploadListReference();
          Render->GetDevice().TransitResourceState(
            currentCommandList,
            OITListsClearBuffer.Resource,
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
        }
      }

      // Clear with 0xFFFFFFFF
      currentCommandList->CopyBufferRegion(
        OITLists.Resource,
        0,
        OITListsClearBuffer.Resource,
        0,
        CurrentOITListsSize);

      //Render->GetDevice().SetCommandListAsUpload(currentCommandList);
      //Render->GetDevice().UpdateBuffer(currentCommandList, OITLists.Resource, OITListsClearVector.data(), CurrentOITListsSize);
      //Render->GetDevice().ClearUploadListReference();



      Render->GetDevice().TransitResourceState(
        currentCommandList,
        OITLists.Resource,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
    PROFILE_END(currentCommandList);
  }
  PROFILE_END(currentCommandList);

  PROFILE_BEGIN(currentCommandList, "Draw Indirect");
  // set common params
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  currentCommandList->SetGraphicsRootConstantBufferView(
    (int)root_parameters_draw_indices::globals_buffer_index,
    Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
  // root_parameters_draw_indices::index_buffer_index will be set via indirect
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::transform_pool_index,
    Render->TransformsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::material_pool_index,
    Render->MaterialsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::texture_pool_index,
    Render->TexturesSystem->TextureTableGPU);
  currentCommandList->SetGraphicsRootUnorderedAccessView(
    (int)root_parameters_draw_indices::oit_lists_index,
    OITLists.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::oit_pool_index,
    OITPoolGPUDescriptor);
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::light_sources_pool_index,
    Render->LightsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::cube_texture_pool_index,
    Render->CubeTexturesSystem->CubeTextureTableGPU);

  currentCommandList->ExecuteIndirect(
    CommandSignature,
    (UINT)Render->IndirectSystem->CPUData.size(),
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    0,
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    Render->IndirectSystem->CounterOffset); // stride to counter

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->IndirectSystem->CommandsUAV[OurUAVIndex].Resource,
    D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_COPY_DEST);
  PROFILE_END(currentCommandList);
  PROFILE_BEGIN(currentCommandList, "Draw Fullscreen rect");
  // after we drawn every object, we need to Draw fullscreen  with special shader
  currentCommandList->SetPipelineState(ComposePSO);
  currentCommandList->SetGraphicsRootSignature(ComposeRootSignature);
  currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

  currentCommandList->SetGraphicsRootConstantBufferView(
    (int)root_parameters_compose_indices::globals_buffer_index,
    Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_compose_indices::oit_lists_index,
    OITLists.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_compose_indices::oit_pool_index,
    OITPool.Resource->GetGPUVirtualAddress());

  currentCommandList->DrawInstanced(3, 1, 0, 0);

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    OITLists.Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

  Render->GetDevice().TransitResourceState(
    currentCommandList,
    OITPool.Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
  PROFILE_END(currentCommandList, "Draw Fullscreen rect");
}


gdr::oit_transparent_pass::~oit_transparent_pass(void)
{
  ComputePSO->Release();
  CommandSignature->Release();
  ComputeRootSignature->Release();
  ComputeShader->Release();

  Render->GetDevice().ReleaseGPUResource(OITPool);
  if (OITListsClearBuffer.Resource != nullptr)
    Render->GetDevice().ReleaseGPUResource(OITListsClearBuffer);
  Render->GetDevice().ReleaseGPUResource(OITLists);
  Render->GetDevice().ReleaseGPUResource(ScreenVertexBuffer);
  ComposePSO->Release();
  ComposeRootSignature->Release();
  ComposeVertexShader->Release();
  ComposePixelShader->Release();

  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
}
