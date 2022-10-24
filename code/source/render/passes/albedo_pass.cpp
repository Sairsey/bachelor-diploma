#include "p_header.h"

void gdr::albedo_pass::Initialize(void)
{
  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/AlbedoColor.hlsl"), {}, shader_stage::Vertex, &VertexShader);
  Render->GetDevice().CompileShader(_T("bin/shaders/AlbedoColor.hlsl"), {}, shader_stage::Pixel, &PixelShader);

  // 2) Create root signature 
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    params.resize((int)root_parameters_draw_indices::total_root_parameters);
    CD3DX12_DESCRIPTOR_RANGE bindlessTexturesDesc[1];  // Textures Pool
    CD3DX12_DESCRIPTOR_RANGE cubeBindlessTexturesDesc[1];  // Textures Pool


    params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView((int)albedo_buffer_registers::globals_buffer_register);
    params[(int)root_parameters_draw_indices::index_buffer_index].InitAsConstants(sizeof(ObjectIndices) / sizeof(int32_t), (int)albedo_buffer_registers::index_buffer_register);
    params[(int)root_parameters_draw_indices::transform_pool_index].InitAsShaderResourceView((int)albedo_texture_registers::object_transform_pool_register);
    params[(int)root_parameters_draw_indices::material_pool_index].InitAsShaderResourceView((int)albedo_texture_registers::material_pool_register);
    params[(int)root_parameters_draw_indices::light_sources_pool_index].InitAsShaderResourceView((int)albedo_texture_registers::light_sources_pool_register);

    {
      bindlessTexturesDesc[0].BaseShaderRegister = (int)albedo_texture_registers::texture_pool_register;
      bindlessTexturesDesc[0].NumDescriptors = MAX_TEXTURE_AMOUNT;
      bindlessTexturesDesc[0].OffsetInDescriptorsFromTableStart = 0;
      bindlessTexturesDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      bindlessTexturesDesc[0].RegisterSpace = 1;
      
      params[(int)root_parameters_draw_indices::texture_pool_index].InitAsDescriptorTable(1, bindlessTexturesDesc);
    }

    {
      cubeBindlessTexturesDesc[0].BaseShaderRegister = (int)albedo_texture_registers::cube_texture_pool_register;
      cubeBindlessTexturesDesc[0].NumDescriptors = MAX_CUBE_TEXTURE_AMOUNT;
      cubeBindlessTexturesDesc[0].OffsetInDescriptorsFromTableStart = 0;
      cubeBindlessTexturesDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      cubeBindlessTexturesDesc[0].RegisterSpace = 2;

      params[(int)root_parameters_draw_indices::cube_texture_pool_index].InitAsDescriptorTable(1, cubeBindlessTexturesDesc);
    }

    // Texture samplers
    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[2];
    samplerDescs[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
    samplerDescs[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_POINT); 

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
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Render->RenderTargets->TargetParams[(int)render_targets_enum::target_frame_hdr].Format;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    Render->GetDevice().CreatePSO(psoDesc, &PSO);
  }

  // 6) Create Command signature
  {
    Render->GetDevice().GetDXDevice()->CreateCommandSignature(&Render->IndirectSystem->commandSignatureDesc, RootSignature, IID_PPV_ARGS(&CommandSignature));
  }
}

bool CullAABBFrustum(
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

  float4x4 matr = (transform * VP).Transposed();
  for (int corner_idx = 0; corner_idx < 8; corner_idx++)
  {
    if (corners[corner_idx].X * matr[3][0] + corners[corner_idx].Y * matr[3][1] + corners[corner_idx].Z * matr[3][2] + matr[3][3] > 0)
      corners[corner_idx] = corners[corner_idx] * matr;
    else
      corners[corner_idx] = corners[corner_idx] * matr * -1 ;
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

void gdr::albedo_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  static mth::matr4f VP = mth::matr::Identity();
  Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_frame_hdr);
  // Update Globals
  Render->GlobalsSystem->CPUData.CameraPos = Render->PlayerCamera.GetPos();
  Render->GlobalsSystem->CPUData.VP = Render->PlayerCamera.GetVP();

  if (!Render->Params.IsVisibilityLocked)
    VP = Render->GlobalsSystem->CPUData.VP;

  PROFILE_BEGIN(currentCommandList, "Update globals");
  Render->GetDevice().SetCommandListAsUpload(currentCommandList);
  Render->GlobalsSystem->UpdateGPUData(currentCommandList);
  Render->GetDevice().ClearUploadListReference();
  PROFILE_END(currentCommandList);

  // set common params
  currentCommandList->SetPipelineState(PSO);
  currentCommandList->SetGraphicsRootSignature(RootSignature);
  currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  std::vector<gdr::gdr_index> objects_to_draw;

  // composite all transparents
  for (gdr::gdr_index i = 0; i < Render->ObjectSystem->CPUPool.size(); i++)
  {
    int transformIndex = Render->ObjectSystem->CPUPool[i].ObjectTransformIndex;
    bool visible = !Render->Params.IsCulling || CullAABBFrustum(
      VP,
      Render->TransformsSystem->CPUData[transformIndex].transform,
      Render->TransformsSystem->CPUData[transformIndex].minAABB,
      Render->TransformsSystem->CPUData[transformIndex].maxAABB);
    if ((Render->ObjectSystem->CPUPool[i].ObjectParams & OBJECT_PARAMETER_TRANSPARENT) == 0 && visible)
    {
      objects_to_draw.push_back(i);
    }
  }

  // just iterate for every geometry
  for (auto &i : objects_to_draw)
  {
    geometry &geom = Render->GeometrySystem->CPUPool[i];

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


void gdr::albedo_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  Render->RenderTargets->Set(currentCommandList, render_targets_enum::target_frame_hdr);
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
  currentCommandList->SetGraphicsRootShaderResourceView(
    (int)root_parameters_draw_indices::light_sources_pool_index,
    Render->LightsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->SetGraphicsRootDescriptorTable(
    (int)root_parameters_draw_indices::cube_texture_pool_index,
    Render->CubeTexturesSystem->CubeTextureTableGPU);
  currentCommandList->ExecuteIndirect(
      CommandSignature,
      (UINT)Render->IndirectSystem->CPUData.size(),
      Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueCulled].Resource,
      0,
      Render->IndirectSystem->CommandsBuffer[(int)indirect_command_enum::OpaqueCulled].Resource,
      Render->IndirectSystem->CounterOffset); // stride to counter
}

gdr::albedo_pass::~albedo_pass(void)
{
  CommandSignature->Release();
  PSO->Release();
  RootSignature->Release();
  VertexShader->Release();
  PixelShader->Release();
}
