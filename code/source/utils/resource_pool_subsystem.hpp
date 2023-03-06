#ifndef NONE_INDEX
#define NONE_INDEX 0xFFFFFFFF
#endif // !NONE_INDEX

template<typename StoredType, gdr_index_types Type, int ChunkSize>
gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::resource_pool_subsystem(render* Rnd)
{
  Render = Rnd;
  GPUData.Resource = nullptr;
  ChunkMarkings.resize(0);
  StoredSize = 0;
  // added first one so we have at least one resource
  Add();
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
void gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::CreateResource()
{
  // allocate Descriptors for pool
  if (!DescriptorsAllocated)
  {
    Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
    DescriptorsAllocated = true;
  }

  // create GPUResource
  Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ sizeof(StoredType) * CPUData.size() }),
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    GPUData,
    &CPUData[0],
    sizeof(StoredType) * CPUData.size());
  GPUData.Resource->SetName(ResourceName.c_str());

  // Create SRV
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = DXGI_FORMAT_UNKNOWN;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Buffer.NumElements = (UINT)CPUData.size();
  srvDesc.Buffer.StructureByteStride = sizeof(StoredType);
  srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

  Render->GetDevice().GetDXDevice()->CreateShaderResourceView(GPUData.Resource, &srvDesc, CPUDescriptor);

  // Allocate chunks flags
  ChunkMarkings.resize((size_t)ceil(1.0 * CPUData.size() * sizeof(StoredType) / CHUNK_SIZE), false);
  for (int i = 0; i < ChunkMarkings.size(); i++)
    ChunkMarkings[i] = false;

  // Save current size
  StoredSize = CPUData.size();
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
void gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::DeleteResource()
{
  if (GPUData.Resource)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
    GPUData.Resource = nullptr;
  }
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
void gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // iterate all resources that needed deletion
  for (int i = 0; i < CPUData.size(); i++)
    if (PoolRecords[i].IsNeedToBeDeleted)
    {
      PoolRecords[i].IsNeedToBeDeleted = false;
      Render->GetDevice().WaitGPUIdle();

      BeforeRemoveJob(i);
    }

  // little defragmentation
  while (PoolRecords.size() > 0 && !PoolRecords[PoolRecords.size() - 1].IsAlive)
  {
    PoolRecords.pop_back();
    CPUData.pop_back();
  }

  // Do anything we need to do before update
  BeforeUpdateJob(pCommandList);

  // check if we need update at all
  bool NeedUpdate = CPUData.size() > StoredSize;
  for (int i = 0; i < ChunkMarkings.size() && !NeedUpdate; i++)
    if (ChunkMarkings[i])
      NeedUpdate = true;

  // if we do not need any update -> return
  if (!NeedUpdate)
  {
    // Do anything we need to do before update
    AfterUpdateJob(pCommandList);
    return;
  }

  if (CPUData.size() > StoredSize) // if we need to reallocate -> Do it
  {
    Render->GetDevice().WaitAllUploadLists();
    Render->GetDevice().WaitGPUIdle();
    DeleteResource();
    CreateResource();
  }
  else // otherwise we can just update right chunks
  {
    // update chunks
    for (int i = 0; i < ChunkMarkings.size(); i++)
    {
      if (ChunkMarkings[i]) // if some chunk changed
      {
        // probably chunks after this are needed in update as well
        int chunk_amount = 1;
        while (i + chunk_amount < ChunkMarkings.size() && ChunkMarkings[i + chunk_amount])
          chunk_amount++;

        // update chunks in one call if they are stored one after another
        int source_offset = i * CHUNK_SIZE;
        int dataSize = min(sizeof(StoredType) * (int)CPUData.size() - source_offset, CHUNK_SIZE * chunk_amount); // Real size of chunk
        if (dataSize > 0)
          Render->GetDevice().UpdateBufferOffset(pCommandList, GPUData.Resource, source_offset, (byte*)&CPUData[0] + source_offset, dataSize);

        // mark chunks as updated
        for (int j = 0; j < chunk_amount; j++)
          ChunkMarkings[i + j] = false;
      }
    }
  }

  // Do anything we need to do before update
  AfterUpdateJob(pCommandList);
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
void gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::UpdateResourceState(ID3D12GraphicsCommandList* pCommandList, bool IsRender)
{
  if (IsRender)
  {
    // Transit state to needed one
    Render->GetDevice().TransitResourceState(
      pCommandList,
      GPUData.Resource,
      D3D12_RESOURCE_STATE_COPY_DEST, UsedResourceState);
  }
  else
  {
    // Transit state to CopyDest
    Render->GetDevice().TransitResourceState(
      pCommandList,
      GPUData.Resource,
      UsedResourceState, D3D12_RESOURCE_STATE_COPY_DEST);
  }

  AfterResourceStateUpdateJob(pCommandList, IsRender);
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
gdr_index gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::Add()
{
  gdr_index Result(NONE_INDEX, Type);
  
  // Try to reuse old resources
  for (gdr_index i = 0; i < CPUData.size() && Result == NONE_INDEX; i++)
    if (!PoolRecords[i].IsAlive)
    {
      Result = i;
      PoolRecords[Result].IsAlive = true;
      PoolRecords[Result].IsNeedToBeDeleted = false;
    }

  // Otherwise we need to create new
  if (Result == NONE_INDEX)
  {
    Result = (unsigned)CPUData.size();
    CPUData.emplace_back();
    PoolRecords.emplace_back();
    PoolRecords[Result].IsAlive = true;
    PoolRecords[Result].IsNeedToBeDeleted = false;
  }

  return Result;
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
void gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::Remove(gdr_index index)
{
  if (!IsExist(index))
    return;

  if (PoolRecords[index].ReferenceCount > 0)
  {
    PoolRecords[index].ReferenceCount--;
  }

  if (PoolRecords[index].ReferenceCount == 0)
  {
    PoolRecords[index].IsAlive = false;
    PoolRecords[index].IsNeedToBeDeleted = true;
  }
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
StoredType& gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::GetEditable(gdr_index index)
{
  GDR_ASSERT((index.type== gdr_index_types::none || index.type == Type) && index <= CPUData.size() && index >= 0 && PoolRecords[index].IsAlive);

  MarkChunkByElementIndex(index);
  return CPUData[index];
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
const StoredType& gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::Get(gdr_index index) const
{
  GDR_ASSERT((index.type == gdr_index_types::none || index.type == Type) && index <= CPUData.size() && index >= 0 && PoolRecords[index].IsAlive);

  return CPUData[index];
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
bool gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::IsExist(gdr_index index) const
{
  return ((index.type == gdr_index_types::none || index.type == Type) && index >= 0 && index < CPUData.size() && PoolRecords[index].IsAlive);
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
void gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::MarkChunkByElementIndex(gdr_index index)
{
  size_t chunk_index = (size_t)floor(1.0 * ((byte*)&CPUData[index] - (byte*)&CPUData[0]) / CHUNK_SIZE);
  if (chunk_index < ChunkMarkings.size() && chunk_index >= 0 && PoolRecords[index].IsAlive)
    ChunkMarkings[chunk_index] = true;
}

template<typename StoredType, gdr_index_types Type, int ChunkSize>
gdr::resource_pool_subsystem<StoredType, Type, ChunkSize>::~resource_pool_subsystem()
{
  DeleteResource();
}