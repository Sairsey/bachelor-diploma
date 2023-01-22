#include "p_header.h"

void gdr::visibility_occlusion_pass::Initialize(void)
{
  // 1) Compile our shader
  Render->GetDevice().CompileShader(_T("bin/shaders/visibility/occlusion.hlsl"), {}, shader_stage::Compute, &ComputeShader);

  // 2) Create root signature for frustum compute
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    CD3DX12_DESCRIPTOR_RANGE descr[3] = {};

    params.resize((int)root_parameters_occlusion_indices::total_root_parameters);

    params[(int)root_parameters_occlusion_indices::compute_globals_index].InitAsConstants(sizeof(GDRGPUComputeGlobals) / sizeof(int32_t), GDRGPUComputeGlobalDataConstantBufferSlot);
    params[(int)root_parameters_occlusion_indices::object_transform_pool_index].InitAsShaderResourceView(GDRGPUObjectTransformPoolSlot);
    params[(int)root_parameters_occlusion_indices::all_commands_pool_index].InitAsShaderResourceView(GDRGPUAllCommandsPoolSlot);

    {
      descr[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUOpaqueCulledCommandsPoolSlot);
      params[(int)root_parameters_occlusion_indices::opaque_culled_commands_pool_index].InitAsDescriptorTable(1, &descr[0]);
    }

    {
      descr[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUTransparentsCulledCommandsPoolSlot);
      params[(int)root_parameters_occlusion_indices::transparent_culled_commands_pool_index].InitAsDescriptorTable(1, &descr[1]);
    }

    {
      descr[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, GDRGPUHierDepthSlot);
      params[(int)root_parameters_occlusion_indices::hier_depth_index].InitAsDescriptorTable(1, &descr[2]);
    }

    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[1];
    samplerDescs[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_POINT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 1, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
  }

  // 3) Create frustum compute PSO
  {
    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = RootSignature;
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

void gdr::visibility_occlusion_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // update ComputeGlobals
  {
    if (!Render->Params.IsViewLocked)
      ComputeGlobals.VP = Render->GlobalsSystem->CPUData.VP;
    ComputeGlobals.width = Render->GlobalsSystem->CPUData.width;
    ComputeGlobals.height = Render->GlobalsSystem->CPUData.height;
    ComputeGlobals.frustumCulling = Render->Params.IsFrustumCulling;
    ComputeGlobals.occlusionCulling = Render->Params.IsOccusionCulling;
    ComputeGlobals.commandsCount = Render->DrawCommandsSystem->CPUData.size();
  }

  // fill Direct emulations of indirect pools
  for (auto& i : Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::All])
  {
    auto& command = Render->DrawCommandsSystem->CPUData[i];

    bool visible = !ComputeGlobals.frustumCulling ||
      CullAABBFrustum(
        ComputeGlobals.VP,
        Render->ObjectTransformsSystem->CPUData[command.Indices.ObjectTransformIndex].Transform,
        Render->ObjectTransformsSystem->CPUData[command.Indices.ObjectTransformIndex].minAABB,
        Render->ObjectTransformsSystem->CPUData[command.Indices.ObjectTransformIndex].maxAABB);;

    bool transparent = command.Indices.ObjectParamsMask & OBJECT_PARAMETER_TRANSPARENT;

    if (visible)
    {
      if (transparent)
        Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::TransparentsCulled].push_back(i);
      else
        Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::OpaqueCulled].push_back(i);
    }
  }

  // Transit resource state of valid buffers to INDIRECT_ARGUMENT.
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueCulled].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::TransparentsCulled].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

void gdr::visibility_occlusion_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // update ComputeGlobals
  {
    if (!Render->Params.IsViewLocked)
      ComputeGlobals.VP = Render->GlobalsSystem->CPUData.VP;
    ComputeGlobals.width = Render->GlobalsSystem->CPUData.width;
    ComputeGlobals.height = Render->GlobalsSystem->CPUData.height;
    ComputeGlobals.frustumCulling = Render->Params.IsFrustumCulling;
    ComputeGlobals.occlusionCulling = Render->Params.IsOccusionCulling;
    ComputeGlobals.commandsCount = Render->DrawCommandsSystem->CPUData.size();
  }

  currentCommandList->SetPipelineState(ComputePSO);

  ID3D12DescriptorHeap* pDescriptorHeaps = Render->GetDevice().GetDescriptorHeap();
  currentCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);
  currentCommandList->SetComputeRootSignature(RootSignature);

  currentCommandList->SetComputeRoot32BitConstants(
    (int)root_parameters_occlusion_indices::compute_globals_index, // root parameter index
    sizeof(GDRGPUComputeGlobals) / sizeof(int32_t),
    &ComputeGlobals,
    0);
  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_occlusion_indices::object_transform_pool_index,
    Render->ObjectTransformsSystem->GPUData.Resource->GetGPUVirtualAddress());
  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_occlusion_indices::all_commands_pool_index,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::All].Resource->GetGPUVirtualAddress());
  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_occlusion_indices::hier_depth_index,
    Render->RenderTargetsSystem->HierDepthGPUDescriptorHandle);
  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_occlusion_indices::opaque_culled_commands_pool_index,
    Render->DrawCommandsSystem->CommandsGPUDescriptor[(int)indirect_command_pools_enum::OpaqueCulled]);
  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_occlusion_indices::transparent_culled_commands_pool_index,
    Render->DrawCommandsSystem->CommandsGPUDescriptor[(int)indirect_command_pools_enum::TransparentsCulled]);

  currentCommandList->Dispatch(static_cast<UINT>(ceil(ComputeGlobals.commandsCount / float(GDRGPUComputeThreadBlockSize))), 1, 1);

  // Transit resource state of valid buffers to INDIRECT_ARGUMENT.
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueCulled].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::TransparentsCulled].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

gdr::visibility_occlusion_pass::~visibility_occlusion_pass(void)
{
  ComputePSO->Release();
  RootSignature->Release();
  ComputeShader->Release();
}
