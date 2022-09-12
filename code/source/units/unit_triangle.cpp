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
  Triangle = Engine->ObjectSystem->CreateObject(vertices, sizeof(vertices) / sizeof(vertices[0]), indices, sizeof(indices) / sizeof(indices[0]));
  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_triangle::Response(void)
{
  float ratio_x = 1, ratio_y = 1;
  float ProjSize = 1;
  float ProjDist = 0.1; 
  float FarClip = 1000;
  auto q = Engine->GetSize();
  UINT W = q.first;
  UINT H = q.second;

  if (W >= H)
    ratio_x = (float)W / H;
  else
    ratio_y = (float)H / W;

  Engine->GlobalsSystem->CPUData.time = 1.0 * clock() / CLOCKS_PER_SEC;
  Engine->GlobalsSystem->CPUData.VP = mth::matr4f::Identity();
  
  Engine->GlobalsSystem->CPUData.VP = 
    mth::matr4f::View(mth::vec3f(0, 0.5, -1), mth::vec3f(0, 0, 0), mth::vec3f(0, 1, 0)) *
  mth::matr4f::Frustum(
    -ratio_x * ProjSize / 2,
    ratio_x * ProjSize / 2,
    -ratio_y * ProjSize / 2,
    ratio_y * ProjSize / 2,
    ProjDist, FarClip);
  Engine->GlobalsSystem->CPUData.VP = Engine->GlobalsSystem->CPUData.VP.Transposed();
  Engine->ObjectSystem->GetTransforms(Triangle).transform = mth::matr::Translate(mth::vec3f(0, 0, sin(Engine->GlobalsSystem->CPUData.time))).Transposed();
}