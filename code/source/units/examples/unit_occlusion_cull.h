#pragma once
#include "../unit_base.h"

class unit_occlusion_cull : public gdr::unit_base
{
private:
  std::vector<gdr_index> Models;
  std::vector<mth::matr4f> Translations;
  const int halfBoxSize = 10;
  const float squareRootOfTotalCount = 100; // around 10000 objects
public:
  void Initialize(void)
  {
    auto import_data = gdr::ImportMeshAssimp("bin/models/bread/bread.obj");
    
    ID3D12GraphicsCommandList* commandList;
    
    for (int i = 0; i < squareRootOfTotalCount; i++)
    {
      // thi is huge data copy, so clear ring buffer every step
      Engine->GetDevice().WaitAllUploadLists();

      Engine->GetDevice().BeginUploadCommandList(&commandList);
      PROFILE_BEGIN(commandList, "unit_occlusion_cull Init");
      for (int j = 0; j < squareRootOfTotalCount; j++)
      {
        mth::vec3f pos = { 1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX };
        pos = pos * 2 - mth::vec3f(1, 1, 1);
        pos *= halfBoxSize;
        gdr_index ModelIndex = Engine->AddModel(import_data);
        mth::matr4f Transform = mth::matr::Translate(pos);
        Models.push_back(ModelIndex);
        Translations.push_back(Transform);
        
        gdr_index ModelRootTransform = Engine->ModelsPool[ModelIndex].Rnd.RootTransform;
        Engine->ObjectTransformsSystem->CPUData[ModelRootTransform].Transform = Transform;
      }
      PROFILE_END(commandList);
      Engine->GetDevice().CloseUploadCommandList();
    }
  }

  void Response(void)
  {
  }

  std::string GetName(void)
  {
    return "unit_occlusion_cull";
  }

  ~unit_occlusion_cull(void)
  {
  }
};