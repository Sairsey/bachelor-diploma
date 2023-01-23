#ifdef NONE_INDEX
#undef NONE_INDEX
#endif // NONE_INDEX
#define NONE_INDEX 0xFFFFFFFF

template<typename StoredType>
gdr::resource_pool_subsystem<StoredType, 0>::resource_pool_subsystem(render* Rnd)
{
  Render = Rnd;
}

template<typename StoredType>
gdr_index gdr::resource_pool_subsystem<StoredType, 0>::Add()
{
  gdr_index Result = NONE_INDEX;

  // Try to reuse old resources
  for (gdr_index i = 0; i < CPUData.size(); i++)
    if (!PoolRecords[i].IsAlive)
      Result = i;

  // Otherwise we need to create new
  if (Result == NONE_INDEX)
  {
    Result = CPUData.size();
    CPUData.emplace_back();
    PoolRecords.emplace_back();
    PoolRecords[Result].IsAlive = true;
  }

  return Result;
}

template<typename StoredType>
void gdr::resource_pool_subsystem<StoredType, 0>::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // Do anything we need to do before update
  BeforeUpdateJob(pCommandList);

  // Do anything we need to do before update
  AfterUpdateJob(pCommandList);
}

template<typename StoredType>
void gdr::resource_pool_subsystem<StoredType, 0>::Remove(gdr_index index)
{
  if (index > CPUData.size() || index < 0 || !PoolRecords[index].IsAlive)
    printf("ERROR");
  PoolRecords[index].IsAlive = false;

  // little defragmentation
  while(!PoolRecords[PoolRecords.size() - 1].IsAlive)
  {
    PoolRecords.pop_back();
    CPUData.pop_back();
  }
}

template<typename StoredType>
StoredType& gdr::resource_pool_subsystem<StoredType, 0>::GetEditable(gdr_index index)
{
  assert(index < CPUData.size() && index >= 0 && PoolRecords[index].IsAlive);
  if (index > CPUData.size() || index < 0 || !PoolRecords[index].IsAlive)
    printf("ERROR");
  return CPUData[index];
}

template<typename StoredType>
const StoredType& gdr::resource_pool_subsystem<StoredType, 0>::Get(gdr_index index) const
{
  return CPUData[index];
}

template<typename StoredType>
bool gdr::resource_pool_subsystem<StoredType, 0>::IsExist(gdr_index index) const
{
  return (index >= 0 && index < CPUData.size() && PoolRecords[index].IsAlive);
}

template<typename StoredType>
gdr::resource_pool_subsystem<StoredType, 0>::~resource_pool_subsystem()
{
}