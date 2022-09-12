#include "p_header.h"

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
  Triangle = Engine->ObjectSystem->CreateObject(vertices, sizeof(vertices), indices, sizeof(indices));
  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_triangle::Response(void)
{
  Engine->GlobalsSystem->CPUData.time = 1.0 * clock() / CLOCKS_PER_SEC;
}