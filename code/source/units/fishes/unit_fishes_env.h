#pragma once
#include "../unit_base.h"

class unit_fishes_env : public gdr::unit_base
{
private:
  std::vector<gdr_index> SceneModels;
  std::vector<gdr_index> ScenePhysics;
  std::vector<gdr_index> LampLights;
  mth::vec3f LampColor = { 10, 10, 10 };
public:
  void Initialize(void)
  {
    auto SceneImportModels = gdr::ImportSplittedModelFromAssimp("bin\\models\\ManyFish\\gallery\\gallery.glb");

    
    // Lamps positions
    std::vector<mth::matr4f> LampTransforms;
    LampTransforms.push_back(mth::matr4f::Translate({-5.5f, 3.8f, -1.5f}));
    LampTransforms.push_back(mth::matr4f::Translate({ 10.3f, 3.8f, -1.5f }));

    // Load stage
    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_thriller_stage Load stage");
    for (int i = 0; i < SceneImportModels.size(); i++)
    {
      SceneModels.push_back(Engine->ModelsManager->Add(SceneImportModels[i]));
      ScenePhysics.push_back(Engine->PhysicsManager->AddStaticMesh(SceneImportModels[i]));
    }
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();


    // Load Lights
    for (int i = 0; i < LampTransforms.size(); i++)
    {
      // create Light
      LampLights.push_back(Engine->LightsSystem->Add());

      gdr_index LightIndex = LampLights[LampLights.size() - 1];

      // Init light
      Engine->LightsSystem->GetEditable(LightIndex).Color = LampColor;
      Engine->LightsSystem->GetEditable(LightIndex).LightSourceType = LIGHT_SOURCE_TYPE_SPOT;
      Engine->LightsSystem->GetEditable(LightIndex).ConstantAttenuation = 1.0f;
      Engine->LightsSystem->GetEditable(LightIndex).LinearAttenuation = 0.09f;
      Engine->LightsSystem->GetEditable(LightIndex).QuadricAttenuation = 0.032f;
      Engine->LightsSystem->GetEditable(LightIndex).AngleInnerCone = 45 * MTH_D2R;
      Engine->LightsSystem->GetEditable(LightIndex).AngleOuterCone = 60 * MTH_D2R;
      Engine->LightsSystem->GetEditable(LightIndex).ShadowMapIndex = Engine->ShadowMapsSystem->Add(2048, 2048);
      Engine->LightsSystem->GetEditable(LightIndex).ShadowMapOffset = 0.0001f;

      // Set light position
      Engine->LightsSystem->GetEditable(LightIndex).ObjectTransformIndex = Engine->ObjectTransformsSystem->Add();
      Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->GetEditable(LightIndex).ObjectTransformIndex).Transform = LampTransforms[i];
    }
  }

  void Response(void)
  {
      Engine->PhysicsManager->IsInterpolating = false;
  }

  std::string GetName(void)
  {
    return "unit_fishes_env";
  }

  ~unit_fishes_env(void)
  {
    for (int i = 0; i < SceneModels.size(); i++)
    {
      Engine->ModelsManager->Remove(SceneModels[i]);
      Engine->PhysicsManager->Remove(ScenePhysics[i]);
    }
    for (int i = 0; i < LampLights.size(); i++)
    {
      Engine->LightsSystem->Remove(LampLights[i]);
    }
  }
};