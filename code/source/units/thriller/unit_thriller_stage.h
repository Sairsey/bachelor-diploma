#pragma once
#include "../unit_base.h"

class unit_thriller_stage : public gdr::unit_base
{
private:
  std::vector<gdr_index> SceneModels;
  std::vector<gdr_index> LampModels;
  std::vector<gdr_index> LampLights;
  mth::vec3f LampColor = {10, 10, 10};
public:
  void Initialize(void)
  {
    auto SceneImportModels = gdr::ImportSplittedModelFromAssimp("bin\\models\\Thriller\\scene\\scene.glb");
    auto LightImportModel = gdr::ImportModelFromAssimp("bin\\models\\Thriller\\lamp.glb");
    
    // Lamps positions
    std::vector<mth::matr4f> LampTransforms;
    //LampTransforms.push_back(mth::matr4f::Translate({0, 10, 0}));

    // Load stage
    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_thriller_stage Load stage");
    for (int i = 0; i < SceneImportModels.size(); i++)
    {
      SceneModels.push_back(Engine->ModelsManager->Add(SceneImportModels[i]));
    }
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();


    // Load Lights
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_thriller_stage Load lights");
    for (int i = 0; i < LampTransforms.size(); i++)
    {
      // create Model
      LampModels.push_back(Engine->ModelsManager->Add(LightImportModel));
      // create Light
      LampLights.push_back(Engine->LightsSystem->Add());
      
      gdr_index LightIndex = LampLights[LampLights.size() - 1];
      gdr_index ModelIndex = LampModels[LampModels.size() - 1];

      // Init light
      Engine->LightsSystem->GetEditable(LightIndex).Color = LampColor;
      Engine->LightsSystem->GetEditable(LightIndex).LightSourceType = LIGHT_SOURCE_TYPE_SPOT;
      Engine->LightsSystem->GetEditable(LightIndex).ConstantAttenuation = 1.0f;
      Engine->LightsSystem->GetEditable(LightIndex).LinearAttenuation = 0.09f;
      Engine->LightsSystem->GetEditable(LightIndex).QuadricAttenuation = 0.032f;
      Engine->LightsSystem->GetEditable(LightIndex).AngleInnerCone = 45;
      Engine->LightsSystem->GetEditable(LightIndex).AngleOuterCone = 60;

      // Bind light to Model position
      Engine->LightsSystem->GetEditable(LightIndex).ObjectTransformIndex = Engine->ModelsManager->Get(ModelIndex).Render.RootTransform;
      Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->GetEditable(LightIndex).ObjectTransformIndex).Transform = LampTransforms[i];
    }
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();    
  }

  void Response(void)
  {
  }

  std::string GetName(void)
  {
    return "unit_thriller_stage";
  }

  ~unit_thriller_stage(void)
  {
    for (int i = 0; i < SceneModels.size(); i++)
    {
      Engine->ModelsManager->Remove(SceneModels[i]);
    }
    for (int i = 0; i < LampLights.size(); i++)
    {
      Engine->ModelsManager->Remove(LampModels[i]);
      Engine->LightsSystem->Remove(LampLights[i]);
    }
  }
};