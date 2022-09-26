#include "p_header.h"

#include "unit_pbr_spheres.h"

#include <thread>
#include <future>

void unit_pbr_spheres::Initialize(void)
{
  ID3D12GraphicsCommandList* commandList;
  double alpha = 0;
  for (int j = 0; j < 40; j++)
  {
    // thi is huge data copy, so clear ring buffer every step
    Engine->GetDevice().WaitAllUploadLists();

    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_pbr_spheres Init");
    for (int i = 0; i < 1000; i++)
    {
      Spheres.push_back(Engine->ObjectSystem->CreateObjectsFromFile("bin/models/pbr_sphere/pbr_sphere.glb")[0]);
      int k = Spheres.size() - 1;

      double dist = (Engine->ObjectSystem->GetTransforms(Spheres[k]).maxAABB - Engine->ObjectSystem->GetTransforms(Spheres[k]).minAABB).Lenght();
      alpha = sqrt(alpha * alpha + dist);
      double radius = alpha * 2;
      Engine->ObjectSystem->GetTransforms(Spheres[k]).transform = mth::matr::Translate(mth::vec3f(float(sin(alpha) * radius), 0, float(cos(alpha) * radius)));
    }
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }
}

void unit_pbr_spheres::Response(void)
{
}