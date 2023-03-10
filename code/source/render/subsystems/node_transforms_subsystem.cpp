#include "p_header.h"

void gdr::node_transforms_subsystem::BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList)
{
  // first we check if we need to update any transform
  PROFILE_CPU_BEGIN("Recalculate Node Transforms")
    for (int i = 0; i < AllocatedSize(); i++)
      if (IsExist(i) && Get(i).IsNeedRecalc)
        UpdateHierarchy(i);
  PROFILE_CPU_END();
}

void gdr::node_transforms_subsystem::UpdateHierarchy(gdr_index index)
{
  if (Get(index).ParentIndex == NONE_INDEX)
    GetEditable(index).GlobalTransform = GetEditable(index).LocalTransform;
  else
    GetEditable(index).GlobalTransform = Get(index).LocalTransform * Get(Get(index).ParentIndex).GlobalTransform;
  GetEditable(index).IsNeedRecalc = false;
  int updateIndex = Get(index).ChildIndex;
  while (updateIndex != NONE_INDEX)
  {
    UpdateHierarchy(updateIndex);
    updateIndex = Get(updateIndex).NextIndex;
  }
}

gdr_index gdr::node_transforms_subsystem::Add(gdr_index parent)
{
  gdr_index Result = resource_pool_subsystem::Add();
  GDRGPUNodeTransform& newRecord = GetEditable(Result);

  newRecord.LocalTransform = mth::matr::Identity();
  newRecord.GlobalTransform = mth::matr::Identity();
  newRecord.BoneOffset = mth::matr::Identity();
  newRecord.ChildIndex = NONE_INDEX;
  newRecord.NextIndex = NONE_INDEX;
  newRecord.ParentIndex = parent;

  if (IsExist(parent)) // add to begin
  {
    newRecord.NextIndex = Get(parent).ChildIndex;
    GetEditable(parent).ChildIndex = Result;
  }

  return Result;
}

void gdr::node_transforms_subsystem::BeforeRemoveJob(gdr_index node)
{
  gdr_index parentIndex = CPUData[node].ParentIndex;
  // if we have parent
  if (parentIndex != NONE_INDEX)
  {
    gdr_index childIndex = CPUData[parentIndex].ChildIndex;
    if (childIndex == node) // and we are the first child of that parent
    {
        CPUData[parentIndex].ChildIndex = CPUData[node].NextIndex; // set next child as first
    }
    else // if not first
    {
      // walk to previous
      while (CPUData[childIndex].NextIndex != node || CPUData[childIndex].NextIndex != NONE_INDEX)
        childIndex = CPUData[childIndex].NextIndex;

      // set next for previous as our next
      if (CPUData[childIndex].NextIndex != NONE_INDEX)
        CPUData[childIndex].NextIndex = CPUData[node].NextIndex;
    }
  }
  
  // for our childs
  gdr_index childIndex = CPUData[node].ChildIndex;
  while (childIndex != NONE_INDEX)
  {
    gdr_index nextChildIndex = CPUData[childIndex].NextIndex;
    CPUData[childIndex].ParentIndex = NONE_INDEX;
    CPUData[childIndex].NextIndex = NONE_INDEX;
    childIndex = nextChildIndex;
  }

  // now we can delete it in normal way;
}
