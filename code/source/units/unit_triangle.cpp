#include "p_header.h"

#if 0
#include "unit_triangle.h"
void unit_triangle::Initialize(void)
{
  static const gdr::vertex vertices[3] = {
  {{-0.5f, -0.5f, 0}, {1,0,0}, {0, 0}},
  {{ 0, 0.5f, 0}, {0,0,1} , {0, 0}},
  {{ 0.5f, -0.5f, 0}, {0,1,0} , {0, 0}}
  };
  static const UINT32 indices[3] = { 0, 1, 2 };
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "unit_triangle Init");
  Triangle = Engine->ObjectSystem->CreateObject(vertices, sizeof(vertices) / sizeof(vertices[0]), indices, sizeof(indices) / sizeof(indices[0]));
  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_triangle::Response(void)
{
  Engine->ObjectSystem->NodesPool[Triangle].GetTransformEditable() = mth::matr::Translate(mth::vec3f(0, 0, sin(Engine->GetTime()) / 2.0f));
}
#endif