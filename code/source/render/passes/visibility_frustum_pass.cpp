#include "p_header.h"

void gdr::visibility_frustum_pass::Initialize(void)
{
  // 1) Compile our shader
  Render->GetDevice().CompileShader(_T("bin/shaders/visibility/frustum.hlsl"), {}, shader_stage::Compute, &ComputeShader);

  // 2) Create root signature for frustum compute
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    CD3DX12_DESCRIPTOR_RANGE descr[3] = {};

    params.resize((int)root_parameters_frustum_indices::total_root_parameters);

    params[(int)root_parameters_frustum_indices::compute_globals_index].InitAsConstants(sizeof(GDRGPUComputeGlobals) / sizeof(int32_t), GDRGPUComputeGlobalDataConstantBufferSlot);
    params[(int)root_parameters_frustum_indices::object_transform_pool_index].InitAsShaderResourceView(GDRGPUObjectTransformPoolSlot);
    params[(int)root_parameters_frustum_indices::all_commands_pool_index].InitAsShaderResourceView(GDRGPUAllCommandsPoolSlot);
    
    {
      descr[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUOpaqueAllCommandsPoolSlot);
      params[(int)root_parameters_frustum_indices::opaque_all_commands_pool_index].InitAsDescriptorTable(1, &descr[0]);
    }

    {
      descr[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUTransparentAllCommandsPoolSlot);
      params[(int)root_parameters_frustum_indices::transparents_all_commands_pool_index].InitAsDescriptorTable(1, &descr[1]);
    }

    {
      descr[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, GDRGPUOpaqueFrustumCommandsPoolSlot);
      params[(int)root_parameters_frustum_indices::opaque_frustum_commands_pool_index].InitAsDescriptorTable(1, &descr[2]);
    }

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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

void gdr::visibility_frustum_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  // update ComputeGlobals
  {
    if (!Render->Params.IsViewLocked)
      ComputeGlobals.VP = Render->GlobalsSystem->Get().VP;
    ComputeGlobals.width = Render->GlobalsSystem->Get().Width;
    ComputeGlobals.height = Render->GlobalsSystem->Get().Height;
    ComputeGlobals.frustumCulling = Render->Params.IsFrustumCulling;
    ComputeGlobals.occlusionCulling = Render->Params.IsOccusionCulling;
    ComputeGlobals.commandsCount = (UINT)Render->DrawCommandsSystem->AllocatedSize();
  }

  // fill Direct emulations of indirect pools
  for (auto& i : Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::All])
  {
    if (!Render->DrawCommandsSystem->IsExist(i))
      continue;
    auto& command = Render->DrawCommandsSystem->Get(i);
    
    bool visible = !ComputeGlobals.frustumCulling ||
      CullAABBFrustum(
        ComputeGlobals.VP,
        Render->ObjectTransformsSystem->Get(command.Indices.ObjectTransformIndex).Transform,
        Render->ObjectTransformsSystem->Get(command.Indices.ObjectTransformIndex).minAABB,
        Render->ObjectTransformsSystem->Get(command.Indices.ObjectTransformIndex).maxAABB);

    bool transparent = command.Indices.ObjectParamsMask & OBJECT_PARAMETER_TRANSPARENT;
    bool topmost = command.Indices.ObjectParamsMask & OBJECT_PARAMETER_TOP_MOST;

    if (!topmost)
    {
        if (transparent)
            Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::TransparentAll].push_back(i);
        else
            Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::OpaqueAll].push_back(i);

        if (!transparent && visible)
            Render->DrawCommandsSystem->DirectCommandPools[(int)indirect_command_pools_enum::OpaqueFrustrumCulled].push_back(i);
    }
  }

  // Transit resource state of valid buffers to INDIRECT_ARGUMENT.
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueAll].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::TransparentAll].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueFrustrumCulled].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

void gdr::visibility_frustum_pass::CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  currentCommandList->SetPipelineState(ComputePSO);

  ID3D12DescriptorHeap* pDescriptorHeaps = Render->GetDevice().GetDescriptorHeap();
  currentCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);
  currentCommandList->SetComputeRootSignature(RootSignature);

  // update ComputeGlobals
  {
    if (!Render->Params.IsViewLocked)
      ComputeGlobals.VP = Render->GlobalsSystem->Get().VP;
    ComputeGlobals.width = Render->GlobalsSystem->Get().Width;
    ComputeGlobals.height = Render->GlobalsSystem->Get().Height;
    ComputeGlobals.frustumCulling = Render->Params.IsFrustumCulling;
    ComputeGlobals.occlusionCulling = Render->Params.IsOccusionCulling;
    ComputeGlobals.commandsCount = (UINT)Render->DrawCommandsSystem->AllocatedSize();
  }

  currentCommandList->SetComputeRoot32BitConstants(
    (int)root_parameters_frustum_indices::compute_globals_index, // root parameter index
    sizeof(GDRGPUComputeGlobals) / sizeof(int32_t),
    &ComputeGlobals,
    0);
  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_frustum_indices::object_transform_pool_index,
    Render->ObjectTransformsSystem->GetGPUResource().Resource->GetGPUVirtualAddress());
  currentCommandList->SetComputeRootShaderResourceView(
    (int)root_parameters_frustum_indices::all_commands_pool_index,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::All].Resource->GetGPUVirtualAddress());
  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_frustum_indices::opaque_all_commands_pool_index,
    Render->DrawCommandsSystem->CommandsGPUDescriptor[(int)indirect_command_pools_enum::OpaqueAll]);
  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_frustum_indices::transparents_all_commands_pool_index,
    Render->DrawCommandsSystem->CommandsGPUDescriptor[(int)indirect_command_pools_enum::TransparentAll]);
  currentCommandList->SetComputeRootDescriptorTable(
    (int)root_parameters_frustum_indices::opaque_frustum_commands_pool_index,
    Render->DrawCommandsSystem->CommandsGPUDescriptor[(int)indirect_command_pools_enum::OpaqueFrustrumCulled]);

  currentCommandList->Dispatch(static_cast<UINT>(ceil(ComputeGlobals.commandsCount / float(GDRGPUComputeThreadBlockSize))), 1, 1);

  // Transit resource state of valid buffers to INDIRECT_ARGUMENT.
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueAll].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::TransparentAll].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  Render->GetDevice().TransitResourceState(
    currentCommandList,
    Render->DrawCommandsSystem->CommandsBuffer[(int)indirect_command_pools_enum::OpaqueFrustrumCulled].Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

gdr::visibility_frustum_pass::~visibility_frustum_pass(void)
{
  RootSignature->Release();
  ComputePSO->Release();
  ComputeShader->Release();
}
