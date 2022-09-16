#include "p_header.h"

#include "unit_bread.h"

void unit_bread::Initialize(void)
{
  ID3D12GraphicsCommandList* commandList;
  for (int j = 0; j < 10; j++)
  {
    // thi is huge data copy, so clear ring buffer every step
    Engine->GetDevice().WaitAllUploadLists();

    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_bread Init");
    for (int i = 0; i < 1000; i++)
      Bread.push_back(Engine->ObjectSystem->CreateObjectsFromFile("bin/models/Bread/Bread.obj")[0]);
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }
}

void unit_bread::Response(void)
{

  double alpha = 0;
  for (int i = 0; i < Bread.size(); i++)
  {
    double dist = (Engine->ObjectSystem->GetTransforms(Bread[i]).maxAABB - Engine->ObjectSystem->GetTransforms(Bread[i]).minAABB).Lenght();
    alpha = sqrt(alpha * alpha + dist);
    double radius = alpha * 2;
    mth::matr translation = mth::matr::Translate(mth::vec3f(float(sin(alpha) * radius), 0, float(cos(alpha) * radius)));
    
    Engine->ObjectSystem->GetTransforms(Bread[i]).transform = mth::matr::RotateY(Engine->GetTime() * 10.0f) * translation;
  }
}