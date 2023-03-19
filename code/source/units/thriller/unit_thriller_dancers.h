#pragma once
#include "../unit_base.h"

class unit_thriller_dancers : public gdr::unit_base
{
private:  
           // index in Animations, index in ModelsPool
  std::vector<std::pair<gdr_index, gdr_index>> DancersModels;
  std::vector<gdr_index> Animations;
public:
  void Initialize(void)
  {
    // list all supported models
    std::vector<std::string> AvaliableModels;
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\aku\\aku.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\barry\\barry.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\boog\\boog.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\burger_king\\burger_king.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\chungus\\chungus.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\crash\\crash.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\dr_doof\\dr_doof.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\garfield\\garfield.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\heavy\\heavy.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\isabelle\\isabelle.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\jake\\jake.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\killer_bean\\killer_bean.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\kira\\kira.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\kronk\\kronk.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\robotnik\\robotnik.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\sans\\sans.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\squidward\\squidward.glb");
    AvaliableModels.push_back("bin\\models\\Thriller\\dancers\\spy\\spy.glb");


    // list all root transforms 
    std::vector<mth::matr4f> RootTransforms;
    RootTransforms.push_back(mth::matr4f::RotateY(20) * mth::matr4f::Translate({-27, -1.5f, 8}));
    RootTransforms.push_back(mth::matr4f::RotateY(20) * mth::matr4f::Translate({ -30, -1.2f, 2 }));
    RootTransforms.push_back(mth::matr4f::RotateY(20) * mth::matr4f::Translate({ -17, 2, -13.5f }));
    RootTransforms.push_back(mth::matr4f::RotateY(100) * mth::matr4f::Translate({ -8.8f, -0.5f, -0.3f }));
    RootTransforms.push_back(mth::matr4f::RotateY(180) * mth::matr4f::Translate({ -6.7f, -1.5f, 22 }));
    RootTransforms.push_back(mth::matr4f::RotateY(180) * mth::matr4f::Translate({ 8.9f, -1.6f, 22 }));
    RootTransforms.push_back(mth::matr4f::RotateY(40) * mth::matr4f::Translate({ 28.9f, -1.2f, 13 }));
    RootTransforms.push_back(mth::matr4f::RotateY(140) * mth::matr4f::Translate({ 41, -1.5f, 23 }));
    RootTransforms.push_back(mth::matr4f::RotateY(40) * mth::matr4f::Translate({ 41, -1.4f, 13 }));
    RootTransforms.push_back(mth::matr4f::RotateY(40) * mth::matr4f::Translate({ 67, 1.37f, 11 }));
    RootTransforms.push_back(mth::matr4f::RotateY(140) * mth::matr4f::Translate({ 64, 1.29f, 25 }));
    RootTransforms.push_back(mth::matr4f::RotateY(120) * mth::matr4f::Translate({ 91.8f, 1.2f, 26 }));
    RootTransforms.push_back(mth::matr4f::RotateY(90) * mth::matr4f::Translate({ 112, 3.5f, 22 }));
    RootTransforms.push_back(mth::matr4f::RotateY(180) * mth::matr4f::Translate({ 98, 1.6f, 33 }));
    RootTransforms.push_back(mth::matr4f::RotateY(180) * mth::matr4f::Translate({ 110, 1.6f, 43 }));
    RootTransforms.push_back(mth::matr4f::RotateY(180) * mth::matr4f::Translate({ 105, 1.6f, 55 }));
    RootTransforms.push_back(mth::matr4f::RotateY(180) * mth::matr4f::Translate({ 110, 2.5f, 71 }));
    RootTransforms.push_back(mth::matr4f::RotateY(-90) * mth::matr4f::Translate({ 75, 1.5f, 62 }));
    RootTransforms.push_back(mth::matr4f::RotateY(-90) * mth::matr4f::Translate({ 34, -1.5f, 49.2f }));
    
    for (int i = 28; i > 12; i -= 2)
    {
      RootTransforms.push_back(mth::matr4f::RotateY(-110) * mth::matr4f::Translate({ 1.0f * i, -1.8f, 53 }));
      RootTransforms.push_back(mth::matr4f::RotateY(-70) * mth::matr4f::Translate({ 1.0f * i, -1.8f, 45 }));
      RootTransforms.push_back(mth::matr4f::RotateY(-110) * mth::matr4f::Translate({ 1.0f * i, -1.8f, 51.5f }));
      RootTransforms.push_back(mth::matr4f::RotateY(-70) * mth::matr4f::Translate({ 1.0f * i, -1.8f, 46.5f }));
      
      RootTransforms.push_back(mth::matr4f::RotateY(-110) * mth::matr4f::Translate({ 1.0f * i - 1, -1.8f, 52.25f }));
      RootTransforms.push_back(mth::matr4f::RotateY(-70) * mth::matr4f::Translate({ 1.0f * i - 1, -1.8f, 45.75f }));
    }

    for (int i = 12; i > 0; i -= 2)
    {
      // i == 0 -> -2.3
      // i == 12 -> -1.8
      float height = (- 1.8f) * (i / 12.0f) + (-2.4f) * (1 - i / 12.0f);
      RootTransforms.push_back(mth::matr4f::RotateY(-110) * mth::matr4f::Translate({ 1.0f * i, height, 53 }));
      RootTransforms.push_back(mth::matr4f::RotateY(-70) * mth::matr4f::Translate({ 1.0f * i, height, 45 }));
      RootTransforms.push_back(mth::matr4f::RotateY(-110) * mth::matr4f::Translate({ 1.0f * i, height, 51.5f }));
      RootTransforms.push_back(mth::matr4f::RotateY(-70) * mth::matr4f::Translate({ 1.0f * i, height, 46.5f }));

      RootTransforms.push_back(mth::matr4f::RotateY(-110) * mth::matr4f::Translate({ 1.0f * i - 1, height, 52.25f }));
      RootTransforms.push_back(mth::matr4f::RotateY(-70) * mth::matr4f::Translate({ 1.0f * i - 1, height, 45.75f }));
    }
    for (int i = 0; i > -30; i -= 2)
    {
      // i == 0 -> -2.4
      // i == -30 -> -5.7
      float height = (-2.4f) * (1 + i / 30.0f) + (-5.7f) * (-i / 30.0f);
      RootTransforms.push_back(mth::matr4f::RotateY(-110) * mth::matr4f::Translate({ 1.0f * i, height, 53 }));
      RootTransforms.push_back(mth::matr4f::RotateY(-70) * mth::matr4f::Translate({ 1.0f * i, height, 45 }));
      RootTransforms.push_back(mth::matr4f::RotateY(-110) * mth::matr4f::Translate({ 1.0f * i, height, 51.5f }));
      RootTransforms.push_back(mth::matr4f::RotateY(-70) * mth::matr4f::Translate({ 1.0f * i, height, 46.5f }));

      RootTransforms.push_back(mth::matr4f::RotateY(-110) * mth::matr4f::Translate({ 1.0f * i - 1, height, 52.25 }));
      RootTransforms.push_back(mth::matr4f::RotateY(-70) * mth::matr4f::Translate({ 1.0f * i - 1, height, 45.75 }));
    }

    RootTransforms.push_back(mth::matr4f::RotateY(-90) * mth::matr4f::Translate({ -58, -5.5f, 46 }));
    RootTransforms.push_back(mth::matr4f::RotateY(-90) * mth::matr4f::Translate({ -62, -5.8f, 53 }));
    RootTransforms.push_back(mth::matr4f::RotateY(-90) * mth::matr4f::Translate({ -62, -5.8f, 100 }));
    RootTransforms.push_back(mth::matr4f::RotateY(0) * mth::matr4f::Translate({ -18, -0.5f, 88 }));
    RootTransforms.push_back(mth::matr4f::RotateY(0) * mth::matr4f::Translate({ -17.5f, -1.5f, 66 }));
    RootTransforms.push_back(mth::matr4f::RotateY(0) * mth::matr4f::Translate({ -30, -1.5f, 64 }));

    /// load models
    std::vector<gdr::model_import_data> ImportedModels;
    for (int i = 0; i < AvaliableModels.size(); i++)
      ImportedModels.push_back(gdr::ImportModelFromAssimp(AvaliableModels[i]));
    
    // import animation from first of them
    for (int i = 0; i < AvaliableModels.size(); i++)
      Animations.push_back(Engine->AnimationManager->Add(ImportedModels[i]));
    
    // Load models
    ID3D12GraphicsCommandList* commandList;
    for (int i = 0; i < RootTransforms.size(); i++)
    {
      Engine->GetDevice().BeginUploadCommandList(&commandList);
      PROFILE_BEGIN(commandList, "unit_thriller_dancers load dancers");
      int dancerId = int(1.0f * rand() / RAND_MAX * ImportedModels.size());
      DancersModels.push_back(std::make_pair(dancerId, Engine->ModelsManager->Add(ImportedModels[dancerId])));

      Engine->ObjectTransformsSystem->GetEditable(
        Engine->ModelsManager->Get(
          DancersModels[DancersModels.size() - 1].second
        ).Render.RootTransform
      ).Transform = RootTransforms[i];
      PROFILE_END(commandList);
      Engine->GetDevice().CloseUploadCommandList();
    }

    std::sort(DancersModels.begin(), DancersModels.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
      });
  }

  void Response(void)
  {
    for (int i = 0; i < DancersModels.size(); i++)
      Engine->AnimationManager->SetAnimationTime(DancersModels[i].second, Animations[DancersModels[i].first], Engine->GetTime() * 1000, 5 * 1000);
  }

  std::string GetName(void)
  {
    return "unit_thriller_dancers";
  }

  ~unit_thriller_dancers(void)
  {
    for (int i = 0; i < DancersModels.size(); i++)
      Engine->ModelsManager->Remove(DancersModels[i].second);
    for (int i = 0; i < Animations.size(); i++)
      Engine->AnimationManager->Remove(Animations[i]);
  }
};