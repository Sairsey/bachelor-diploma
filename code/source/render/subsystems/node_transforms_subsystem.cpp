#include "p_header.h"
#include <thread>
#include <future>


gdr::node_transforms_subsystem::node_transforms_subsystem(render* Rnd)
{
  Render = Rnd;
  GPUData.Resource = nullptr;
  ChunkMarkings.resize(0);
  StoredSize = 0;
}

void gdr::node_transforms_subsystem::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // first we check if we need to update any transform
  PROFILE_CPU_BEGIN("Recalculate Node Transforms")
    for (int i = 0; i < CPUData.size(); i++)
      if (CPUData[i].IsNeedRecalc)
        UpdateHierarchy(i);
  PROFILE_CPU_END();

  // if buffers are not the same size - recreate everything
  if (CPUData.size() > StoredSize)
  {
    if (GPUData.Resource != nullptr)
    {
      Render->GetDevice().ReleaseGPUResource(GPUData);
      GPUData.Resource = nullptr;
    }
    Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ sizeof(GDRGPUNodeTransform) * CPUData.size() }),
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      GPUData,
      &CPUData[0],
      sizeof(GDRGPUNodeTransform) * CPUData.size());
    GPUData.Resource->SetName(L"Node Transforms pool");

    Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = (UINT)CPUData.size();
    srvDesc.Buffer.StructureByteStride = sizeof(GDRGPUNodeTransform);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    Render->GetDevice().GetDXDevice()->CreateShaderResourceView(GPUData.Resource, &srvDesc, CPUDescriptor);
    ChunkMarkings.resize((size_t)ceil(1.0 * CPUData.size() * sizeof(GDRGPUNodeTransform) / CHUNK_SIZE), false);

    StoredSize = CPUData.size();
  }
  else
  {
    // check if we need to update anything
    bool NeedUpdate = false;
    for (int i = 0; i < ChunkMarkings.size(); i++)
      if (ChunkMarkings[i]) // if some chunk changed
      {
        NeedUpdate = true;
        break;
      }

    if (NeedUpdate)
    {
      // Change state to COPY_DEST
      Render->GetDevice().TransitResourceState(
        pCommandList,
        GPUData.Resource,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

      // Then update hierarchy
      for (int i = 0; i < ChunkMarkings.size(); i++)
      {
        if (ChunkMarkings[i]) // if some chunk changed
        {
          // probably chunks after this are needed in update as well
          int chunk_amount = 1;
          while (i + chunk_amount < ChunkMarkings.size() && ChunkMarkings[i + chunk_amount])
            chunk_amount++;

          int source_offset = i * CHUNK_SIZE;
          gdr_index dataSize = (gdr_index)min(sizeof(GDRGPUNodeTransform) * CPUData.size() - source_offset, CHUNK_SIZE * chunk_amount); // Real size of chunk

          Render->GetDevice().UpdateBufferOffset(pCommandList, GPUData.Resource, source_offset, (byte*)&CPUData[0] + source_offset, dataSize); // update only 1 chunk
          for (int j = 0; j < chunk_amount; j++)
            ChunkMarkings[i + j] = false;
        }
      }
    }
  }

  // Set state to right one
  Render->GetDevice().TransitResourceState(
    pCommandList,
    GPUData.Resource,
    D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void gdr::node_transforms_subsystem::UpdateHierarchy(gdr_index index)
{
  if (CPUData[index].ParentIndex == NONE_INDEX)
    CPUData[index].GlobalTransform = CPUData[index].LocalTransform;
  else
    CPUData[index].GlobalTransform = CPUData[index].LocalTransform * CPUData[CPUData[index].ParentIndex].GlobalTransform;
  MarkChunkByTransformIndex(index);
  CPUData[index].IsNeedRecalc = false;
  int updateIndex = CPUData[index].ChildIndex;
  while (updateIndex != NONE_INDEX)
  {
    UpdateHierarchy(updateIndex);
    updateIndex = CPUData[updateIndex].NextIndex;
  }
}

gdr_index gdr::node_transforms_subsystem::CreateNode(gdr_index parent)
{
  CPUData.emplace_back();
  GDRGPUNodeTransform& newRecord = CPUData[CPUData.size() - 1];

  newRecord.LocalTransform = mth::matr::Identity();
  newRecord.GlobalTransform = mth::matr::Identity();
  newRecord.BoneOffset = mth::matr::Identity();
  newRecord.ChildIndex = NONE_INDEX;
  newRecord.NextIndex = NONE_INDEX;
  newRecord.ParentIndex = parent;

  if (parent != NONE_INDEX) // add to begin
  {
    newRecord.NextIndex = CPUData[parent].ChildIndex;
    CPUData[parent].ChildIndex = (gdr_index)(CPUData.size() - 1);
  }
  MarkChunkByTransformIndex(CPUData.size() - 1);
  return (gdr_index)(CPUData.size() - 1);
}

void gdr::node_transforms_subsystem::DeleteNode(gdr_index node)
{
  int parentIndex = CPUData[node].ParentIndex;
  if (parentIndex != NONE_INDEX)
  {
    int childIndex = CPUData[parentIndex].ChildIndex;
    if (childIndex == node)
    {
      CPUData[parentIndex].ChildIndex = CPUData[node].NextIndex;
    }
    else
    {
      while (CPUData[childIndex].NextIndex != node || CPUData[childIndex].NextIndex != NONE_INDEX)
        childIndex = CPUData[childIndex].NextIndex;

      if (CPUData[childIndex].NextIndex != NONE_INDEX)
        CPUData[childIndex].NextIndex = CPUData[node].NextIndex;
    }
  }
}

void gdr::node_transforms_subsystem::MarkChunkByTransformIndex(gdr_index index)
{
  gdr_index chunk_index = (gdr_index)(floor(1.0 * ((byte*)&CPUData[index] - (byte*)&CPUData[0]) / CHUNK_SIZE));
  CPUData[index].IsNeedRecalc = true;
  if (chunk_index < ChunkMarkings.size() && chunk_index >= 0)
    ChunkMarkings[chunk_index] = true;
}


gdr::node_transforms_subsystem::~node_transforms_subsystem()
{
  if (GPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
  }
}
