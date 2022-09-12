#include "p_header.h"

void gdr::unit_base::Initialize(void)
{
  static const gdr::vertex vertices[3] = {
  {{-0.5f, -0.5f, 0}, {1,0,0}, {0, 0}},
  {{ 0, 0.5f, 0}, {0,0,1} , {0, 0}},
  {{ 0.5f, -0.5f, 0}, {0,1,0} , {0, 0}}
  };
  static const UINT32 indices[3] = { 0, 1, 2 };
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "unit_base Init");
  Engine->Geometry->CreateGeometry(vertices, sizeof(vertices), indices, sizeof(indices));
  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}