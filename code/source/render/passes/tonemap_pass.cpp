#include "p_header.h"

void gdr::tonemap_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/Tonemap.hlsl"), {}, shader_stage::Vertex, &TonemapVertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/Tonemap.hlsl"), {}, shader_stage::Pixel, &TonemapPixelShader);

  Render->GetDevice().CompileShader(_T("bin/shaders/LuminanceFirst.hlsl"), {}, shader_stage::Vertex, &FirstVertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/LuminanceFirst.hlsl"), {}, shader_stage::Pixel, &FirstPixelShader);

  Render->GetDevice().CompileShader(_T("bin/shaders/LuminanceCopy.hlsl"), {}, shader_stage::Vertex, &CopyVertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/LuminanceCopy.hlsl"), {}, shader_stage::Pixel, &CopyPixelShader);

  Render->GetDevice().CompileShader(_T("bin/shaders/LuminanceFinal.hlsl"), {}, shader_stage::Compute, &LuminanceFinalComputeShader);

  // 2) Create InputLayout
  static const D3D12_INPUT_ELEMENT_DESC combineInputElementDescs[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };

  // 3) Create root signature for copy
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_copy_indices::total_root_parameters);
    D3D12_DESCRIPTOR_RANGE descRanges[1] = {};

    {
      descRanges[0].BaseShaderRegister = (UINT)tonemap_texture_registers::texture_register;
      descRanges[0].NumDescriptors = 1;
      descRanges[0].OffsetInDescriptorsFromTableStart = 0;
      descRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      descRanges[0].RegisterSpace = 0;
    }

    params[(int)root_parameters_copy_indices::source_texture_index].InitAsDescriptorTable(1, descRanges);

    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[1];
    samplerDescs[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 1, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &CopyRootSignature);
  }

  // 4) Create root signature for Luminance final
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_luminance_final_indices::total_root_parameters);
    D3D12_DESCRIPTOR_RANGE descRanges[1] = {};
    D3D12_DESCRIPTOR_RANGE uavDesc[1] = {};

    {
      descRanges[0].BaseShaderRegister = (UINT)tonemap_texture_registers::texture_register;
      descRanges[0].NumDescriptors = 1;
      descRanges[0].OffsetInDescriptorsFromTableStart = 0;
      descRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      descRanges[0].RegisterSpace = 0;
    }

    {
      uavDesc[0].BaseShaderRegister = (UINT)tonemap_uav_registers::luminance_variables_register;
      uavDesc[0].NumDescriptors = 1;
      uavDesc[0].OffsetInDescriptorsFromTableStart = 0;
      uavDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      uavDesc[0].RegisterSpace = 0;
    }

    params[(int)root_parameters_luminance_final_indices::globals_buffer_index].InitAsConstantBufferView((int)tonemap_buffer_registers::globals_buffer_register);
    params[(int)root_parameters_luminance_final_indices::source_texture_index].InitAsDescriptorTable(1, descRanges);
    params[(int)root_parameters_luminance_final_indices::luminance_variables_index].InitAsDescriptorTable(1, uavDesc);

    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[1];
    samplerDescs[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 1, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &LuminanceFinalRootSignature);
  }

  // 5) Create root signature for tonemap
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_tonemap_indices::total_root_parameters);
    D3D12_DESCRIPTOR_RANGE descRanges[1] = {};
    D3D12_DESCRIPTOR_RANGE uavDesc[1] = {};

    {
      descRanges[0].BaseShaderRegister = (UINT)tonemap_texture_registers::texture_register;
      descRanges[0].NumDescriptors = 1;
      descRanges[0].OffsetInDescriptorsFromTableStart = 0;
      descRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      descRanges[0].RegisterSpace = 0;
    }

    {
      uavDesc[0].BaseShaderRegister = (UINT)tonemap_uav_registers::luminance_variables_register;
      uavDesc[0].NumDescriptors = 1;
      uavDesc[0].OffsetInDescriptorsFromTableStart = 0;
      uavDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      uavDesc[0].RegisterSpace = 0;
    }

    params[(int)root_parameters_tonemap_indices::globals_buffer_index].InitAsConstantBufferView((int)tonemap_buffer_registers::globals_buffer_register);
    params[(int)root_parameters_tonemap_indices::hdr_texture_index].InitAsDescriptorTable(1, descRanges);
    params[(int)root_parameters_tonemap_indices::luminance_variables_index].InitAsConstantBufferView((int)tonemap_buffer_registers::luminance_variables_register);

    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[1];
    samplerDescs[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 1, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &TonemapRootSignature);
  }

  // 6.1) Create FirstLuminance PSO
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { combineInputElementDescs, 2 };
    psoDesc.pRootSignature = CopyRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(FirstVertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(FirstPixelShader);
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
    psoDesc.RTVFormats[0] = Render->RenderTargets->TargetParams[(int)render_targets_enum::target_frame_lum_final].Format;
    psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &FirstPSO);
  }

  // 6.1) Create Copy PSO
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { combineInputElementDescs, 2 };
    psoDesc.pRootSignature = CopyRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(CopyVertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(CopyPixelShader);
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
    psoDesc.RTVFormats[0] = Render->RenderTargets->TargetParams[(int)render_targets_enum::target_frame_lum_final].Format;
    psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &CopyPSO);
  }

  // 7) Create LuminanceFinal PSO
  {
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = LuminanceFinalRootSignature;
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(LuminanceFinalComputeShader);

    Render->GetDevice().CreateComputePSO(psoDesc, &LuminanceFinalPSO);
  }

  // 8) Create Tonemap PSO
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { combineInputElementDescs, 2 };
    psoDesc.pRootSignature = TonemapRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(TonemapVertexShader);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(TonemapPixelShader);
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
    psoDesc.RTVFormats[0] = Render->RenderTargets->TargetParams[(int)render_targets_enum::target_display].Format;
    psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &TonemapPSO);
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
    PROFILE_BEGIN(commandList, "tonemap pass create screen mesh");

    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer(5 * sizeof(float) * 6),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        nullptr,
        ScreenVertexBuffer,
        vertices,
        5 * sizeof(float) * 6);
      ScreenVertexBuffer.Resource->SetName(L"Tonemap Screen vertex buffer");
    }
    {
      ScreenVertexBufferView.BufferLocation = ScreenVertexBuffer.Resource->GetGPUVirtualAddress();
      ScreenVertexBufferView.StrideInBytes = (UINT)(5 * sizeof(float));
      ScreenVertexBufferView.SizeInBytes = (UINT)(5 * sizeof(float) * 6);
    }

    PROFILE_END(commandList);
    Render->GetDevice().CloseUploadCommandList();
  }

  // 10) Create Luminance Variables buffer 
  {
    Render->GetDevice().AllocateStaticDescriptors(2, LuminanceBufferCPU, LuminanceBufferGPU);
    Render->GetDevice().CreateGPUResource(
      CD3DX12_RESOURCE_DESC::Buffer(Align(sizeof(LuminanceVariables), (size_t)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
      nullptr,
      LuminanceBuffer);

    LuminanceBuffer.Resource->SetName(L"Luminance data");

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.NumElements = 1;
    uavDesc.Buffer.StructureByteStride = sizeof(LuminanceVariables);
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(LuminanceBuffer.Resource, nullptr, &uavDesc, LuminanceBufferCPU);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc = {};
    cbDesc.BufferLocation = LuminanceBuffer.Resource->GetGPUVirtualAddress();
    cbDesc.SizeInBytes = Align(sizeof(LuminanceVariables), (size_t)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

    D3D12_CPU_DESCRIPTOR_HANDLE descr = LuminanceBufferCPU;
    descr.ptr += Render->GetDevice().GetSRVDescSize();
    Render->GetDevice().GetDXDevice()->CreateConstantBufferView(&cbDesc, descr);
  }
}

void gdr::tonemap_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  {
    PROFILE_BEGIN(currentCommandList, "Draw Luminance Luminance");
    Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_frame_lum);
    currentCommandList->SetPipelineState(FirstPSO);
    currentCommandList->SetGraphicsRootSignature(CopyRootSignature);
    currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

    D3D12_GPU_DESCRIPTOR_HANDLE tmp_descr = Render->RenderTargets->ShaderResourceViewsGPU;
    tmp_descr.ptr += (int)(render_targets_enum::target_frame_hdr) * Render->GetDevice().GetSRVDescSize();
    currentCommandList->SetGraphicsRootDescriptorTable(
      (int)root_parameters_copy_indices::source_texture_index, tmp_descr);

    currentCommandList->DrawInstanced(6, 1, 0, 0);
    
    PROFILE_END(currentCommandList);
  }

  {
    PROFILE_BEGIN(currentCommandList, "Downsample Luminance");
    for (int render_target_enum = (int)render_targets_enum::target_frame_lum_2; render_target_enum <= (int)render_targets_enum::target_frame_lum_final; render_target_enum++)
    {
      Render->RenderTargets->Set(currentCommandList, (render_targets_enum)render_target_enum);
      currentCommandList->SetPipelineState(CopyPSO);
      currentCommandList->SetGraphicsRootSignature(CopyRootSignature);
      currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

      D3D12_GPU_DESCRIPTOR_HANDLE tmp_descr = Render->RenderTargets->ShaderResourceViewsGPU;
      tmp_descr.ptr += (int)(render_target_enum - 1) * Render->GetDevice().GetSRVDescSize();
      currentCommandList->SetGraphicsRootDescriptorTable(
        (int)root_parameters_copy_indices::source_texture_index, tmp_descr);

      currentCommandList->DrawInstanced(6, 1, 0, 0);
    }
    PROFILE_END(currentCommandList);
  }

  Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_display);

  {
    PROFILE_BEGIN(currentCommandList, "final luminance");
    currentCommandList->SetPipelineState(LuminanceFinalPSO);
    currentCommandList->SetComputeRootSignature(LuminanceFinalRootSignature);

    D3D12_GPU_DESCRIPTOR_HANDLE tmp_descr = Render->RenderTargets->ShaderResourceViewsGPU;
    tmp_descr.ptr += (int)(render_targets_enum::target_frame_lum_final) * Render->GetDevice().GetSRVDescSize();
    
    currentCommandList->SetComputeRootConstantBufferView(
      (int)root_parameters_luminance_final_indices::globals_buffer_index, Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());

    Render->GetDevice().TransitResourceState(currentCommandList, LuminanceBuffer.Resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    
    currentCommandList->SetComputeRootDescriptorTable(
        (int)root_parameters_luminance_final_indices::source_texture_index, tmp_descr);

    currentCommandList->SetComputeRootDescriptorTable(
        (int)root_parameters_luminance_final_indices::luminance_variables_index, LuminanceBufferGPU);

    currentCommandList->Dispatch(1, 1, 1);

    Render->GetDevice().TransitResourceState(currentCommandList, LuminanceBuffer.Resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    PROFILE_END(currentCommandList);
  }


  {
    PROFILE_BEGIN(currentCommandList, "Combine to Display");

    // after we drawn every object, we need to Draw fullscreen  with special shader
    currentCommandList->SetPipelineState(TonemapPSO);
    currentCommandList->SetGraphicsRootSignature(TonemapRootSignature);
    currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

    currentCommandList->SetGraphicsRootConstantBufferView(
      (int)root_parameters_tonemap_indices::globals_buffer_index, Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());

    D3D12_GPU_DESCRIPTOR_HANDLE tmp_descr = Render->RenderTargets->ShaderResourceViewsGPU;
    tmp_descr.ptr += (int)render_targets_enum::target_frame_hdr * Render->GetDevice().GetSRVDescSize();
    currentCommandList->SetGraphicsRootDescriptorTable(
      (int)root_parameters_tonemap_indices::hdr_texture_index, tmp_descr);

    currentCommandList->SetGraphicsRootConstantBufferView(
        (int)root_parameters_tonemap_indices::luminance_variables_index, LuminanceBuffer.Resource->GetGPUVirtualAddress());

    currentCommandList->DrawInstanced(6, 1, 0, 0);
    PROFILE_END(currentCommandList);
  }
}

gdr::tonemap_pass::~tonemap_pass(void)
{
  Render->GetDevice().ReleaseGPUResource(ScreenVertexBuffer);
  Render->GetDevice().ReleaseGPUResource(LuminanceBuffer);

  FirstVertexShader->Release();
  FirstPixelShader->Release();

  CopyVertexShader->Release();
  CopyPixelShader->Release();
  LuminanceFinalComputeShader->Release();
  TonemapVertexShader->Release();
  TonemapPixelShader->Release();

  // Root signature
  CopyRootSignature->Release();
  LuminanceFinalRootSignature->Release();
  TonemapRootSignature->Release();

  // Pipeline state object
  FirstPSO->Release();
  CopyPSO->Release();
  LuminanceFinalPSO->Release();
  TonemapPSO->Release();
}
