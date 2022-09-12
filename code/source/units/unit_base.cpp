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
  Engine->Geometry->CreateGeometry(vertices, sizeof(vertices), indices, sizeof(indices));
  Engine->GetDevice().CloseUploadCommandList();
}