#include "p_header.h"

// default constructor
gdr::object_support::object_support(render *Rnd)
{
  Render = Rnd;
}
// function which will import data and return gdr_object
gdr::gdr_object gdr::object_support::CreateObject(const vertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount)
{
  Render->GeometrySystem->CreateGeometry(pVertex, vertexCount, pIndices, indexCount);
  
  ObjectTransform newTransform;

  newTransform.maxAABB = pVertex[0].Pos;
  newTransform.minAABB = pVertex[0].Pos;
  for (int i = 0; i < vertexCount; i++)
  {
    newTransform.maxAABB.X = max(newTransform.maxAABB.X, pVertex[i].Pos.X);
    newTransform.maxAABB.Y = max(newTransform.maxAABB.Y, pVertex[i].Pos.Y);
    newTransform.maxAABB.Z = max(newTransform.maxAABB.Z, pVertex[i].Pos.Z);
    newTransform.minAABB.X = min(newTransform.minAABB.X, pVertex[i].Pos.X);
    newTransform.minAABB.Y = min(newTransform.minAABB.Y, pVertex[i].Pos.Y);
    newTransform.minAABB.Z = min(newTransform.minAABB.Z, pVertex[i].Pos.Z);
  }

  newTransform.transform = mth::matr::Identity();
  newTransform.transformInversedTransposed = mth::matr::Identity();
  Render->TransformsSystem->CPUData.push_back(newTransform);

  ObjectIndices new_record;
  new_record.ObjectTransformIndex = Render->TransformsSystem->CPUData.size() - 1;
  new_record.ObjectMaterialIndex = -1;
  CPUPool.push_back(new_record);
  return CPUPool.size() - 1;
}

ObjectTransform& gdr::object_support::GetTransforms(gdr_object object)
{
  return Render->TransformsSystem->CPUData[object];
}

// destructor
gdr::object_support::~object_support()
{
}