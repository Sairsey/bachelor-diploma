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
  float ProjSize = 0.1;
  float ProjDist = 0.1; 
  float FarClip = 1000;
  auto q = Engine->GetSize();
  UINT W = q.first;
  UINT H = q.second;

  mth::cam3<float> camera(mth::vec3f(0, 0.5, -1), mth::vec3f(0), mth::vec3f(0, 1,0), 0.1, 0.1, 1000.0, W, H);

  Engine->GlobalsSystem->CPUData.time = 1.0 * clock() / CLOCKS_PER_SEC;
  Engine->GlobalsSystem->CPUData.VP = camera.GetVP();
  
  Engine->ObjectSystem->GetTransforms(Triangle).transform = 
    mth::matr::RotateY(Engine->GlobalsSystem->CPUData.time) 
    * mth::matr::Translate(mth::vec3f(0, 0, sin(Engine->GlobalsSystem->CPUData.time) / 2.0));
}