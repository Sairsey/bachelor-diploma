#include "p_header.h"

#include "unit_bread.h"

#include <thread>
#include <future>

void unit_bread::Initialize(void)
{
  ID3D12GraphicsCommandList* commandList;
  double alpha = 0;
  for (int j = 0; j < 40; j++)
  {
    // thi is huge data copy, so clear ring buffer every step
    Engine->GetDevice().WaitAllUploadLists();

    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_bread Init");
    for (int i = 0; i < 1000; i++)
    {
      Bread.push_back(Engine->ObjectSystem->CreateObjectsFromFile("bin/models/Bread/Bread.obj")[0]);
      int k = Bread.size() - 1;

      double dist = (Engine->ObjectSystem->GetTransforms(Bread[k]).maxAABB - Engine->ObjectSystem->GetTransforms(Bread[k]).minAABB).Lenght();
      alpha = sqrt(alpha * alpha + dist);
      double radius = alpha * 2;
      Translations.push_back(mth::matr::Translate(mth::vec3f(float(sin(alpha) * radius), 0, float(cos(alpha) * radius))));
    }
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }
}

void unit_bread::Response(void)
{
  mth::matr rotation = mth::matr::RotateY(Engine->GetTime() * 10.0f);

  /* Parallel Computing matrices
  const auto processor_count = std::thread::hardware_concurrency();
  std::vector<std::future<void>> f;

  for (int thread_i = 0; thread_i < processor_count; thread_i++)
  {
    f.push_back(std::async(std::launch::async, [&](int j) {
      for (int i = j; i < Bread.size(); i += processor_count)
      {
        Engine->ObjectSystem->GetTransforms(Bread[i]).transform = rotation * Translations[i];
      }
      }, thread_i));
  }

  for (int thread_i = 0; thread_i < processor_count; thread_i++)
  {
    f[thread_i].wait();
  }*/

  /* single-thread computing */
  for (int i = 0; i < Bread.size(); i++)
  {
    Engine->ObjectSystem->GetTransforms(Bread[i]).transform = rotation * Translations[i];
  }
}