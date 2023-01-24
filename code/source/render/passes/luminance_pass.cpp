#include "p_header.h"

void gdr::luminance_pass::Initialize(void)
{
    // 1) Compile our shaders
    Render->GetDevice().CompileShader(_T("bin/shaders/luminance/first.hlsl"), {}, shader_stage::Vertex, &FirstVertexShader);
    Render->GetDevice().CompileShader(_T("bin/shaders/luminance/first.hlsl"), {}, shader_stage::Pixel, &FirstPixelShader);
    Render->GetDevice().CompileShader(_T("bin/shaders/luminance/copy.hlsl"), {}, shader_stage::Vertex, &CopyVertexShader);
    Render->GetDevice().CompileShader(_T("bin/shaders/luminance/copy.hlsl"), {}, shader_stage::Pixel, &CopyPixelShader);
    Render->GetDevice().CompileShader(_T("bin/shaders/luminance/final.hlsl"), {}, shader_stage::Compute, &FinalComputeShader);

    // 2) Create InputLayout
    static const D3D12_INPUT_ELEMENT_DESC onScreenInputElementDescs[] =
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // 3) Create root signature for "First" and "Copy"
    {
        std::vector<CD3DX12_ROOT_PARAMETER> params;
        params.resize((int)root_parameters_draw_indices::total_root_parameters);
        D3D12_DESCRIPTOR_RANGE descRanges[1] = {};
        
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
            Render->GetDevice().CreateRootSignature(rootSignatureDesc, &DrawRootSignature);
        }
    }

    // 4) Create root signature for "Final"
    {
      std::vector<CD3DX12_ROOT_PARAMETER> params;
      params.resize((int)root_parameters_final_indices::total_root_parameters);
      D3D12_DESCRIPTOR_RANGE descRanges[1] = {};
      D3D12_DESCRIPTOR_RANGE uavDesc[1] = {};

      {
        descRanges[0].BaseShaderRegister = GDRGPUUserShaderResource1Slot;
        descRanges[0].NumDescriptors = 1;
        descRanges[0].OffsetInDescriptorsFromTableStart = 0;
        descRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRanges[0].RegisterSpace = 0;
        params[(int)root_parameters_final_indices::input_texture_index].InitAsDescriptorTable(1, descRanges);
      }

      {
        uavDesc[0].BaseShaderRegister = GDRGPULuminanceUnorderedAccessBufferSlot;
        uavDesc[0].NumDescriptors = 1;
        uavDesc[0].OffsetInDescriptorsFromTableStart = 0;
        uavDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        uavDesc[0].RegisterSpace = 0;
        params[(int)root_parameters_final_indices::luminance_buffer_index].InitAsDescriptorTable(1, uavDesc);
      }

      params[(int)root_parameters_final_indices::globals_buffer_index].InitAsConstantBufferView(GDRGPUGlobalDataConstantBufferSlot);

      {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init((UINT)params.size(), &params[0], (UINT)Render->TexturesSystem->SamplersDescs.size(), Render->TexturesSystem->SamplersDescs.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        Render->GetDevice().CreateRootSignature(rootSignatureDesc, &ComputeRootSignature);
      }
    }

    // 5) Create PSO for First
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { onScreenInputElementDescs, _countof(onScreenInputElementDescs) };
        psoDesc.pRootSignature = DrawRootSignature;
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
        psoDesc.RTVFormats[0] = Render->RenderTargetsSystem->TargetParams[(int)render_targets_enum::target_frame_lum].Format;
        psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
        psoDesc.SampleDesc.Count = 1;

        Render->GetDevice().CreatePSO(psoDesc, &FirstPSO);
    }

    // 6) Create PSO for Copy
    {
      D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
      psoDesc.InputLayout = { onScreenInputElementDescs, _countof(onScreenInputElementDescs) };
      psoDesc.pRootSignature = DrawRootSignature;
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
      psoDesc.RTVFormats[0] = Render->RenderTargetsSystem->TargetParams[(int)render_targets_enum::target_frame_lum_16].Format;
      psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
      psoDesc.SampleDesc.Count = 1;

      Render->GetDevice().CreatePSO(psoDesc, &CopyPSO);
    }

    // 7) Create PSO for Final
    {
      D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
      psoDesc.pRootSignature = ComputeRootSignature;
      psoDesc.CS = CD3DX12_SHADER_BYTECODE(FinalComputeShader);
      
      Render->GetDevice().CreateComputePSO(psoDesc, &FinalPSO);
    }

    // 8) Create Vertices for Fullscreen draw
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
      PROFILE_BEGIN(commandList, "luminance pass create screen mesh");

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
}

void gdr::luminance_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  {
    PROFILE_BEGIN(currentCommandList, "Draw Luminance in special Render target");
    Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_frame_lum);
    currentCommandList->SetPipelineState(FirstPSO);
    currentCommandList->SetGraphicsRootSignature(DrawRootSignature);
    currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

    D3D12_GPU_DESCRIPTOR_HANDLE tmp_descr = Render->RenderTargetsSystem->ShaderResourceViewsGPU;
    tmp_descr.ptr += (int)(render_targets_enum::target_frame_hdr) * Render->GetDevice().GetSRVDescSize();

    currentCommandList->SetGraphicsRootDescriptorTable(
      (int)root_parameters_draw_indices::input_texture_index, tmp_descr);

    currentCommandList->DrawInstanced(6, 1, 0, 0);

    PROFILE_END(currentCommandList);
  }

  {
    PROFILE_BEGIN(currentCommandList, "Downsample Luminance");
    for (int render_target_enum = (int)render_targets_enum::target_frame_lum_2; render_target_enum <= (int)render_targets_enum::target_frame_lum_final; render_target_enum++)
    {
      Render->RenderTargetsSystem->Set(currentCommandList, (render_targets_enum)render_target_enum);
      currentCommandList->SetPipelineState(CopyPSO);
      currentCommandList->SetGraphicsRootSignature(DrawRootSignature);
      currentCommandList->IASetVertexBuffers(0, 1, &ScreenVertexBufferView);

      // set previous rt as source
      D3D12_GPU_DESCRIPTOR_HANDLE tmp_descr = Render->RenderTargetsSystem->ShaderResourceViewsGPU;
      tmp_descr.ptr += (int)(render_target_enum - 1) * Render->GetDevice().GetSRVDescSize();
      currentCommandList->SetGraphicsRootDescriptorTable(
        (int)root_parameters_draw_indices::input_texture_index, tmp_descr);

      currentCommandList->DrawInstanced(6, 1, 0, 0);
    }
    PROFILE_END(currentCommandList);
  }

  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_display);

  {
    PROFILE_BEGIN(currentCommandList, "Final luminance");
    currentCommandList->SetPipelineState(FinalPSO);
    currentCommandList->SetComputeRootSignature(ComputeRootSignature);

    D3D12_GPU_DESCRIPTOR_HANDLE tmp_descr = Render->RenderTargetsSystem->ShaderResourceViewsGPU;
    tmp_descr.ptr += (int)(render_targets_enum::target_frame_lum_final) * Render->GetDevice().GetSRVDescSize();

    currentCommandList->SetComputeRootConstantBufferView(
      (int)root_parameters_final_indices::globals_buffer_index, Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());

    currentCommandList->SetComputeRootDescriptorTable(
      (int)root_parameters_final_indices::input_texture_index, tmp_descr);

    Render->GetDevice().TransitResourceState(
      currentCommandList,
      Render->LuminanceSystem->GPUData.Resource,
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    currentCommandList->SetComputeRootDescriptorTable(
      (int)root_parameters_final_indices::luminance_buffer_index, Render->LuminanceSystem->GPUDescriptor);

    currentCommandList->Dispatch(1, 1, 1);

    Render->GetDevice().TransitResourceState(
      currentCommandList, 
      Render->LuminanceSystem->GPUData.Resource, 
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    PROFILE_END(currentCommandList);
  }
}

gdr::luminance_pass::~luminance_pass(void)
{
  // Our shaders with PSO
  FirstVertexShader->Release();
  FirstPixelShader->Release();
  FirstPSO->Release();

  CopyVertexShader->Release();
  CopyPixelShader->Release();
  CopyPSO->Release();

  FinalComputeShader->Release();
  FinalPSO->Release();

  DrawRootSignature->Release();
  ComputeRootSignature->Release();

  Render->GetDevice().ReleaseGPUResource(ScreenVertexBuffer);
}