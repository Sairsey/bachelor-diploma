#pragma once
#include "../unit_base.h"

class unit_model_loading : public gdr::unit_base
{
private:
  gdr_index Frog;
public:
  void Initialize(void)
  {
    auto import_data = gdr::ImportModelFromAssimp("bin/models/crazy_frog/crazy_frog.obj");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_model_loading Init");
    Frog = Engine->ModelsManager->Add(import_data);
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }

  void Response(void)
  {
    float radius = 10;
    mth::matr translation = mth::matr::Translate(mth::vec3f(radius, 0, 0));
    Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(Frog).Render.RootTransform).Transform = translation * mth::matr::RotateY(Engine->GetTime() * 21 * 6);
  }

  std::string GetName(void)
  {
    return "unit_model_loading";
  }

  ~unit_model_loading(void)
  {
    Engine->ModelsManager->Remove(Frog);
  }
};