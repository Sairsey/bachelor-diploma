#ifndef NONE_INDEX
#define NONE_INDEX 0xFFFFFFFF
#endif // !NONE_INDEX

template<typename StoredType, gdr_index_types Type>
gdr::resource_pool_subsystem<StoredType, Type, 0>::resource_pool_subsystem(render* Rnd)
{
  Render = Rnd;
}

template<typename StoredType, gdr_index_types Type>
gdr_index gdr::resource_pool_subsystem<StoredType, Type, 0>::Add()
{
  gdr_index Result(NONE_INDEX, Type);

  // Try to reuse old resources
  for (gdr_index i = 0; i < CPUData.size() && Result == NONE_INDEX; i++)
    if (!PoolRecords[i].IsAlive && !PoolRecords[i].IsNeedToBeDeleted)
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

template<typename StoredType, gdr_index_types Type>
void gdr::resource_pool_subsystem<StoredType, Type, 0>::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // iterate all resources that needed deletion
  for (int i = 0; i < CPUData.size(); i++)
    if (PoolRecords[i].IsNeedToBeDeleted)
    {
      PoolRecords[i].IsNeedToBeDeleted = false;
      Render->GetDevice().WaitAllUploadLists();
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

  // Do anything we need to do before update
  AfterUpdateJob(pCommandList);
}

template<typename StoredType, gdr_index_types Type>
void gdr::resource_pool_subsystem<StoredType, Type, 0>::Remove(gdr_index index)
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

template<typename StoredType, gdr_index_types Type>
inline StoredType& gdr::resource_pool_subsystem<StoredType, Type, 0>::GetEditable(gdr_index index)
{
  GDR_ASSERT((index.type == gdr_index_types::none || index.type == Type) && index <= CPUData.size() && index >= 0 && PoolRecords[index].IsAlive);

  return CPUData[index];
}

template<typename StoredType, gdr_index_types Type>
inline const StoredType& gdr::resource_pool_subsystem<StoredType, Type, 0>::Get(gdr_index index) const
{
  GDR_ASSERT((index.type == gdr_index_types::none || index.type == Type) && index <= CPUData.size() && index >= 0 && PoolRecords[index].IsAlive);
  
  return CPUData[index];
}

template<typename StoredType, gdr_index_types Type>
inline bool gdr::resource_pool_subsystem<StoredType, Type, 0>::IsExist(gdr_index index) const
{
  return ((index.type == gdr_index_types::none || index.type == Type) && index >= 0 && index < CPUData.size() && PoolRecords[index].IsAlive);
}

template<typename StoredType, gdr_index_types Type>
gdr::resource_pool_subsystem<StoredType, Type, 0>::~resource_pool_subsystem()
{
}