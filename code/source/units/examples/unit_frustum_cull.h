#pragma once
#include "../unit_base.h"

class unit_frustum_cull : public gdr::unit_base
{
private:
  std::vector<gdr_index> Models;
  std::vector<mth::matr4f> Translations;
  const int halfBoxSize = 10; // around 8000 objects
  const float step = 10;
public:
  void Initialize(void)
  {
    auto import_data = gdr::ImportModelFromAssimp("bin/models/bread/bread.obj");
    
    ID3D12GraphicsCommandList* commandList;
    for (int i = -halfBoxSize; i < halfBoxSize; i++)
    {
      // thi is huge data copy, so clear ring buffer every step
      Engine->GetDevice().WaitAllUploadLists();

      Engine->GetDevice().BeginUploadCommandList(&commandList);
      PROFILE_BEGIN(commandList, "unit_frustum_cull Init");
      for (int j = -halfBoxSize; j < halfBoxSize; j++)
      {
        for (int k = -halfBoxSize; k < halfBoxSize; k++)
        {
          Models.push_back(Engine->ModelsManager->Add(import_data));
          Translations.push_back(
            mth::matr::Translate(
              { i * step, j * step, k * step }));
        }
      }
      PROFILE_END(commandList);
      Engine->GetDevice().CloseUploadCommandList();
    }
  }

  void Response(void)
  {
    mth::matr rotation = mth::matr::RotateY(Engine->GetTime() * 50.0f);

    /* single-thread computing */
    for (int i = 0; i < Models.size(); i++)
    {
      gdr_index ModelIndex = Models[i];
      gdr_index ModelRootTransform = Engine->ModelsManager->Get(ModelIndex).Render.RootTransform;
      Engine->ObjectTransformsSystem->GetEditable(ModelRootTransform).Transform = rotation * Translations[i];
      //Engine->ObjectSystem->NodesPool[Bread[i]].GetTransformEditable() = rotation * Translations[i];
    }
  }

  std::string GetName(void)
  {
    return "unit_frustum_cull";
  }

  ~unit_frustum_cull(void)
  {
  }
};