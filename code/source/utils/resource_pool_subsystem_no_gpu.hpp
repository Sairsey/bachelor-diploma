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
  //gdr_index Result(NONE_INDEX, Type);
  gdr_index Result = NONE_INDEX;

  // Try to reuse old resources
  for (gdr_index i = 0; i < CPUData.size() && Result == NONE_INDEX; i++)
    if (!PoolRecords[i].IsAlive)
    {
      Result = i;
      PoolRecords[Result].IsAlive = true;
    }

  // Otherwise we need to create new
  if (Result == NONE_INDEX)
  {
    Result = (gdr_index)CPUData.size();
    CPUData.emplace_back();
    PoolRecords.emplace_back();
    PoolRecords[Result].IsAlive = true;
  }

  return Result;
}

template<typename StoredType, gdr_index_types Type>
void gdr::resource_pool_subsystem<StoredType, Type, 0>::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
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
    BeforeRemoveJob(index);
    PoolRecords[index].IsAlive = false;

    // little defragmentation
    while (PoolRecords.size() > 0 && !PoolRecords[PoolRecords.size() - 1].IsAlive)
    {
      PoolRecords.pop_back();
      CPUData.pop_back();
    }
  }
}

template<typename StoredType, gdr_index_types Type>
StoredType& gdr::resource_pool_subsystem<StoredType, Type, 0>::GetEditable(gdr_index index)
{
  GDR_ASSERT(/*Type == index.type && */index <= CPUData.size() && index >= 0 && PoolRecords[index].IsAlive);

  return CPUData[index];
}

template<typename StoredType, gdr_index_types Type>
const StoredType& gdr::resource_pool_subsystem<StoredType, Type, 0>::Get(gdr_index index) const
{
  GDR_ASSERT(/*Type == index.type && */index <= CPUData.size() && index >= 0 && PoolRecords[index].IsAlive);
  
  return CPUData[index];
}

template<typename StoredType, gdr_index_types Type>
bool gdr::resource_pool_subsystem<StoredType, Type, 0>::IsExist(gdr_index index) const
{
  return (/*Type == index.type && */ index >= 0 && index < CPUData.size() && PoolRecords[index].IsAlive);
}

template<typename StoredType, gdr_index_types Type>
gdr::resource_pool_subsystem<StoredType, Type, 0>::~resource_pool_subsystem()
{
}