#include "p_header.h"

void gdr::order_transparent_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/TransparentColor.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/TransparentColor.hlsl"), {}, shader_stage::Pixel, &PixelShader);

  // 2) Create root signature 
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);
    CD3DX12_DESCRIPTOR_RANGE bindlessTexturesDesc[1];  // Textures Pool
    CD3DX12_DESCRIPTOR_RANGE cubeBindlessTexturesDesc[1];  // Textures Pool

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
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &PSO);
  }
}

void gdr::order_transparent_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // Update Globals
  Render->GlobalsSystem->CPUData.CameraPos = Render->PlayerCamera.GetPos();
  Render->GlobalsSystem->CPUData.VP = Render->PlayerCamera.GetVP();

  PROFILE_BEGIN(currentCommandList, "Update globals");
  Render->GetDevice().SetCommandListAsUpload(currentCommandList);
  Render->GlobalsSystem->UpdateGPUData(currentCommandList);
  Render->GetDevice().ClearUploadListReference();
  PROFILE_END(currentCommandList);

  // set common params
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  std::vector<gdr::gdr_object> objects_to_draw;

  // composite all transparents
  for (gdr::gdr_object i = 0; i < Render->ObjectSystem->CPUPool.size(); i++)
  {
    if (Render->ObjectSystem->CPUPool[i].ObjectParams & OBJECT_PARAMETER_TRANSPARENT)
    {
      objects_to_draw.push_back(i);
    }
  }

  // Sort them
  std::sort(objects_to_draw.begin(), objects_to_draw.end(), 
    [&](const gdr::gdr_object& a, const gdr::gdr_object& b)
    {
      ObjectTransform &a_transform = Render->ObjectSystem->GetTransforms(a);
      ObjectTransform &b_transform = Render->ObjectSystem->GetTransforms(b);
      mth::vec3f a_position = (a_transform.maxAABB + a_transform.minAABB) / 2.0;
      mth::vec3f b_position = (b_transform.maxAABB + b_transform.minAABB) / 2.0;

      mth::vec3f cam_pos = Render->GlobalsSystem->CPUData.CameraPos;


      return (a_position - cam_pos).Lenght() > (b_position - cam_pos).Lenght();
    });

  for (auto &i : objects_to_draw)
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
      currentCommandList->SetGraphicsRootShaderResourceView(
        (int)root_parameters_draw_indices::light_sources_pool_index,
        Render->LightsSystem->GPUData.Resource->GetGPUVirtualAddress());
      currentCommandList->SetGraphicsRootDescriptorTable(
        (int)root_parameters_draw_indices::cube_texture_pool_index,
        Render->CubeTexturesSystem->CubeTextureTableGPU);
    }

    currentCommandList->DrawIndexedInstanced(geom.IndexCount, 1, 0, 0, 0);
  }
}

gdr::order_transparent_pass::~order_transparent_pass(void)
{
  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
}
