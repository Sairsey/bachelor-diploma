#include "p_header.h"

gdr::indirect_support::indirect_support(render* Rnd)
{
  Render = Rnd;
  CommandsUAVReset.Resource = nullptr;
  CommandsSRV.Resource = nullptr;
  for (int i = 0; i < TotalUAV; i++)
    CommandsUAV[i].Resource = nullptr;
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

void gdr::indirect_support::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // if we added or removed objects
  if (CPUData.size() != Render->ObjectSystem->CPUPool.size())
  {
    // free data
    for (int i = 0; i < TotalUAV; i++)
      if (CommandsUAV[i].Resource != nullptr)
      {
        Render->GetDevice().ReleaseGPUResource(CommandsUAV[i]);
      }
    if (CommandsSRV.Resource != nullptr)
    {
      Render->GetDevice().ReleaseGPUResource(CommandsSRV);
    }

    // fill CPUData
    CPUData.resize(Render->ObjectSystem->CPUPool.size());
    for (int i = 0; i < Render->ObjectSystem->CPUPool.size(); i++)
    {
      CPUData[i].Indices = Render->ObjectSystem->CPUPool[i];
      CPUData[i].VertexBuffer = Render->GeometrySystem->CPUPool[i].VertexBufferView;
      CPUData[i].IndexBuffer = Render->GeometrySystem->CPUPool[i].IndexBufferView;

      CPUData[i].DrawArguments.IndexCountPerInstance = Render->GeometrySystem->CPUPool[i].IndexCount;
      CPUData[i].DrawArguments.InstanceCount = 1;
      CPUData[i].DrawArguments.BaseVertexLocation = 0;
      CPUData[i].DrawArguments.StartIndexLocation = 0;
      CPUData[i].DrawArguments.StartInstanceLocation = 0;
    }

    Render->GetDevice().AllocateStaticDescriptors(1, CommandsSRVCPUDescriptor, CommandsSRVGPUDescriptor);
    // Create SRV
    {
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer({ CPUData.size() * sizeof(indirect_command) }),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        CommandsSRV,
        &CPUData[0],
        CPUData.size() * sizeof(indirect_command));

      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Buffer.NumElements = CPUData.size();
      srvDesc.Buffer.StructureByteStride = sizeof(indirect_command);
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(CommandsSRV.Resource, &srvDesc, CommandsSRVCPUDescriptor);
      CommandsSRV.Resource->SetName(L"CommandsSRV");
    }


    // aligned UAV size
    UINT UAVSize = AlignForUavCounter(CPUData.size() * sizeof(indirect_command));
    CounterOffset = UAVSize;

    // CreateUAVs
    for (int i = 0; i < TotalUAV; i++)
    {
      Render->GetDevice().AllocateStaticDescriptors(1, CommandsUAVCPUDescriptor[i], CommandsUAVGPUDescriptor[i]);
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer({ UAVSize + sizeof(UINT) }, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        CommandsUAV[i]);

      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
      uavDesc.Format = DXGI_FORMAT_UNKNOWN;
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
      uavDesc.Buffer.FirstElement = 0;
      uavDesc.Buffer.NumElements = CPUData.size();
      uavDesc.Buffer.StructureByteStride = sizeof(indirect_command);
      uavDesc.Buffer.CounterOffsetInBytes = UAVSize;
      uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

      Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(CommandsUAV[i].Resource, CommandsUAV[i].Resource, &uavDesc, CommandsUAVCPUDescriptor[i]);
      CommandsUAV[i].Resource->SetName(L"CommandsUAV");
    }

    // Create Reset data
    {
      UINT data = 0;
      // create buffer big enough for all commands
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer({ sizeof(UINT) }),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        CommandsUAVReset,
        &data,
        sizeof(UINT));
    }
  }
}

gdr::indirect_support::~indirect_support()
{
  for (int i = 0; i < TotalUAV; i++)
    if (CommandsUAV[i].Resource != nullptr)
    {
      Render->GetDevice().ReleaseGPUResource(CommandsUAV[i]);
    }
  if (CommandsSRV.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(CommandsSRV);
  }

  if (CommandsUAVReset.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(CommandsUAVReset);
  }
}
