#include "p_header.h"

gdr::draw_commands_subsystem::draw_commands_subsystem(render* Rnd)
{
  Render = Rnd;
  CommandsUAVReset.Resource = nullptr;
  for (int i = 0; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
  {
    CommandsBuffer[i].Resource = nullptr;
    Render->GetDevice().AllocateStaticDescriptors(1, CommandsCPUDescriptor[i], CommandsGPUDescriptor[i]);
  }

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
}

// We pack the UAV counter into the same buffer as the commands rather than create
// a separate 64K resource/heap for it. The counter must be aligned on 4K boundaries,
// so we pad the command buffer (if necessary) such that the counter will be placed
// at a valid location in the buffer.
static UINT AlignForUavCounter(UINT bufferSize)
{
  const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
  return (bufferSize + (alignment - 1)) & ~(alignment - 1);
}

void gdr::draw_commands_subsystem::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
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
    }
  }

  if (CPUData.size() == 0)
    return;

  // if we added or removed objects
  if (CPUData.size() != SavedSize)
  {
    SavedSize = CPUData.size();
    // free data
    for (int i = 0; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
      if (CommandsBuffer[i].Resource != nullptr)
      {
        Render->GetDevice().ReleaseGPUResource(CommandsBuffer[i]);
      }

    // Create SRV
    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer({ CPUData.size() * sizeof(GDRGPUIndirectCommand) }),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        CommandsBuffer[(int)indirect_command_pools_enum::All],
        &CPUData[0],
        CPUData.size() * sizeof(GDRGPUIndirectCommand));
      CommandsBuffer[(int)indirect_command_pools_enum::All].Resource->SetName(L"All commands SRV");

      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Buffer.NumElements = (UINT)CPUData.size();
      srvDesc.Buffer.StructureByteStride = sizeof(GDRGPUIndirectCommand);
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(CommandsBuffer[(int)indirect_command_pools_enum::All].Resource, &srvDesc, CommandsCPUDescriptor[(int)indirect_command_pools_enum::All]);
    }

    // aligned UAV size
    UINT UAVSize = AlignForUavCounter((UINT)CPUData.size() * sizeof(GDRGPUIndirectCommand));
    CounterOffset = UAVSize;

    // CreateUAVs
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

      // Reset UAV count 
      pCommandList->CopyBufferRegion(
        CommandsBuffer[i].Resource,
        CounterOffset,
        CommandsUAVReset.Resource,
        0,
        sizeof(UINT));
    }
  }
  
  // transit state from COPY_DEST to Unordered ACCESS
  for (int i = 1; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
  {
    Render->GetDevice().TransitResourceState(
      pCommandList,
      CommandsBuffer[i].Resource,
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  }
}

gdr::draw_commands_subsystem::~draw_commands_subsystem()
{
  for (int i = 0; i < (int)indirect_command_pools_enum::TotalBuffers; i++)
    if (CommandsBuffer[i].Resource != nullptr)
      Render->GetDevice().ReleaseGPUResource(CommandsBuffer[i]);

  if (CommandsUAVReset.Resource != nullptr)
    Render->GetDevice().ReleaseGPUResource(CommandsUAVReset);
}
