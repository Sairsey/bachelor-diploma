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
  ObjectIndices new_record;
  new_record.ObjectTransformIndex = -1;
  new_record.ObjectMaterialIndex = -1;
  CPUPool.push_back(new_record);
  return CPUPool.size() - 1;
}

// destructor
gdr::object_support::~object_support()
{
}