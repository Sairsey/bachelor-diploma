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

gdr::draw_commands_subsystem::draw_commands_subsystem(render* Rnd) : resource_pool_subsystem(Rnd)
{
  // Additional buffer for Commands
  CommandsUAVReset.Resource = nullptr;
  for (int i = 1; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
  {
    CommandsBuffer[i].Resource = nullptr;
    Render->GetDevice().AllocateStaticDescriptors(1, CommandsCPUDescriptor[i], CommandsGPUDescriptor[i]);
  }

  // Fill command signature Desc
  argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
  argumentDescs[0].Constant.RootParameterIndex = GDRGPUObjectIndicesRecordRootIndex; // because Root parameter 0 is descriptor handle
  argumentDescs[0].Constant.Num32BitValuesToSet = sizeof(GDRGPUObjectIndices) / sizeof(int32_t);

  argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
  argumentDescs[1].VertexBuffer.Slot = 0;

  argumentDescs[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;

  argumentDescs[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

  commandSignatureDesc.pArgumentDescs = argumentDescs;
  commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
  commandSignatureDesc.ByteStride = sizeof(GDRGPUIndirectCommand);

  // fill data specific to resource_pool_subsystem
  resource_pool_subsystem::ResourceName = L"All commands pool";
  //resource_pool_subsystem::UsedResourceState = D3D12_RESOURCE_STATE_COMMON;
}

void gdr::draw_commands_subsystem::BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList)
{
  if (CommandsUAVReset.Resource == nullptr)
  {
    // Create Reset data
    {
      UINT data = 0;
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer({ sizeof(UINT) }),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        CommandsUAVReset,
        &data,
        sizeof(UINT));

      CommandsUAVReset.Resource->SetName(L"CommandsUAVReset");
      Render->GetDevice().TransitResourceState(
        pCommandList,
        CommandsUAVReset.Resource,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
    }
  }
}

void gdr::draw_commands_subsystem::AfterUpdateJob(ID3D12GraphicsCommandList* pCommandList)
{
  // copy SRV to our structure
  CommandsBuffer[(int)indirect_command_pools_enum::All] = GetGPUResource();
  CommandsCPUDescriptor[(int)indirect_command_pools_enum::All] = CPUDescriptor;
  CommandsGPUDescriptor[(int)indirect_command_pools_enum::All] = GPUDescriptor;

  if (CPUData.size() == 0)
    return;

  if (CPUData.size() > UAVStoredSize) // if we added some commands
  {
    // SRV already reallocated.
    UAVStoredSize = CPUData.size();
    DirectCommandPools[(int)indirect_command_pools_enum::All].clear();
    DirectCommandPools[(int)indirect_command_pools_enum::All].reserve(CPUData.size());
    for (int i = 0; i < CPUData.size(); i++)
      DirectCommandPools[(int)indirect_command_pools_enum::All].push_back(i);

    // free data
    for (int i = 1; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
      if (CommandsBuffer[i].Resource != nullptr)
        Render->GetDevice().ReleaseGPUResource(CommandsBuffer[i]);

    // aligned UAV size
    UINT UAVSize = AlignForUavCounter((UINT)CPUData.size() * sizeof(GDRGPUIndirectCommand));
    CounterOffset = UAVSize;

    // Create UAVs
    for (int i = 1; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer({ UAVSize + sizeof(UINT) }, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        CommandsBuffer[i]);
      CommandsBuffer[i].Resource->SetName((L"CommandsUAV " + std::to_wstring(i)).c_str());

      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
      uavDesc.Format = DXGI_FORMAT_UNKNOWN;
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
      uavDesc.Buffer.FirstElement = 0;
      uavDesc.Buffer.NumElements = (UINT)CPUData.size();
      uavDesc.Buffer.StructureByteStride = sizeof(GDRGPUIndirectCommand);
      uavDesc.Buffer.CounterOffsetInBytes = UAVSize;
      uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

      Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(CommandsBuffer[i].Resource, CommandsBuffer[i].Resource, &uavDesc, CommandsCPUDescriptor[i]);

      DirectCommandPools[i].clear();
      // Also 

    }
  }

  for (int i = 1; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
  {
    // Reset UAV count 
    pCommandList->CopyBufferRegion(
      CommandsBuffer[i].Resource,
      CounterOffset,
      CommandsUAVReset.Resource,
      0,
      sizeof(UINT));
    DirectCommandPools[i].clear();
  }

}

void gdr::draw_commands_subsystem::AfterResourceStateUpdateJob(ID3D12GraphicsCommandList* pCommandList, bool IsRender)
{
  if (IsRender)
  {
    // transit state from COPY_DEST to Unordered ACCESS
    for (int i = 1; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
    {
      Render->GetDevice().TransitResourceState(
        pCommandList,
        CommandsBuffer[i].Resource,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
  }
  else
  {
    for (int i = 1; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
    {
      // transit state from indirect argument to CopyDest
      Render->GetDevice().TransitResourceState(
        pCommandList,
        CommandsBuffer[i].Resource,
        D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_COPY_DEST);
    }
  }
}

// Add one element to pool in correct way
gdr_index gdr::draw_commands_subsystem::Add(gdr_index geometryIndex, gdr_index transformIndex, gdr_index materialIndex, gdr_index boneMappingIndex)
{
  gdr_index Result = resource_pool_subsystem::Add();
  GDRGPUIndirectCommand &DrawCommand = GetEditable(Result);

  DrawCommand.Indices.ObjectIndex = geometryIndex;
  DrawCommand.Indices.ObjectParamsMask = 0;
  DrawCommand.Indices.ObjectTransformIndex = transformIndex;
  DrawCommand.Indices.ObjectMaterialIndex = materialIndex;
  DrawCommand.Indices.BoneMappingIndex = boneMappingIndex;

  Render->ObjectTransformsSystem->IncreaseReferenceCount(transformIndex);
  Render->MaterialsSystem->IncreaseReferenceCount(materialIndex);
  Render->BoneMappingSystem->IncreaseReferenceCount(boneMappingIndex);

  DrawCommand.VertexBuffer = Render->GeometrySystem->Get(geometryIndex).VertexBufferView;
  DrawCommand.IndexBuffer = Render->GeometrySystem->Get(geometryIndex).IndexBufferView;

  DrawCommand.DrawArguments.IndexCountPerInstance = Render->GeometrySystem->Get(geometryIndex).IndexCount;
  DrawCommand.DrawArguments.InstanceCount = 1;
  DrawCommand.DrawArguments.BaseVertexLocation = 0;
  DrawCommand.DrawArguments.StartIndexLocation = 0;
  DrawCommand.DrawArguments.StartInstanceLocation = 0;
  DrawCommand.IsExist = true;

  return Result;
}

void gdr::draw_commands_subsystem::BeforeRemoveJob(gdr_index index)
{
  if (IsExist(index))
  {
    GetEditable(index).IsExist = false;
    Render->GeometrySystem->Remove(Get(index).Indices.ObjectIndex);
    Render->MaterialsSystem->Remove(Get(index).Indices.ObjectMaterialIndex);
    Render->BoneMappingSystem->Remove(Get(index).Indices.BoneMappingIndex);
    Render->ObjectTransformsSystem->Remove(Get(index).Indices.ObjectTransformIndex);
  }
}

gdr::draw_commands_subsystem::~draw_commands_subsystem()
{
  // SRV already flushed
  for (int i = 1; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
    if (CommandsBuffer[i].Resource != nullptr)
      Render->GetDevice().ReleaseGPUResource(CommandsBuffer[i]);

  if (CommandsUAVReset.Resource != nullptr)
    Render->GetDevice().ReleaseGPUResource(CommandsUAVReset);
}
