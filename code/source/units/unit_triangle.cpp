#include "p_header.h"

#include "unit_triangle.h"
void unit_triangle::Initialize(void)
{
  static const GDRVertex vertices[3] = {
  {{-0.5f, -0.5f, 0}, {0,0,1}, {0, 0}, {0, 0, 0}, {NONE_INDEX, NONE_INDEX, NONE_INDEX, NONE_INDEX}, {0, 0, 0, 0}},
  {{ 0, 0.5f, 0}, {0,0,1} , {0, 0}, {0, 0, 0}, {NONE_INDEX, NONE_INDEX, NONE_INDEX, NONE_INDEX}, {0, 0, 0, 0}},
  {{ 0.5f, -0.5f, 0}, {0,0,1} , {0, 0}, {0, 0, 0}, {NONE_INDEX, NONE_INDEX, NONE_INDEX, NONE_INDEX}, {0, 0, 0, 0}}
  };
  static const UINT32 indices[3] = { 0, 1, 2 };
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "unit_triangle Init");
  {
	  TriangleGeometry = Engine->GeometrySystem->CPUData.size();
	  Engine->GeometrySystem->CreateGeometry(vertices, 3, indices, 3);
  }
  {
	  TriangleTransform = Engine->ObjectTransformsSystem->CPUData.size();
	  Engine->ObjectTransformsSystem->CPUData.emplace_back();
	  Engine->ObjectTransformsSystem->CPUData[TriangleTransform].Transform = mth::matr::Identity();
  }
  {
	  TriangleDrawCall = Engine->DrawCommandsSystem->AddElementInPool(Engine->GeometrySystem->CPUData[TriangleGeometry]);
	  Engine->DrawCommandsSystem->CPUData[TriangleDrawCall].Indices.ObjectTransformIndex = TriangleTransform;
  }
  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_triangle::Response(void)
{
  Engine->ObjectTransformsSystem->CPUData[TriangleTransform].Transform = mth::matr::Translate(mth::vec3f(0, 0, sin(Engine->GetTime()) / 2.0f));
  Engine->ObjectTransformsSystem->MarkChunkByTransformIndex(TriangleTransform);
}