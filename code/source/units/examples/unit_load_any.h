#pragma once
#include "../unit_base.h"

class unit_load_any : public gdr::unit_base
{
private:
  gdr_index Model = NONE_INDEX;
  gdr_index Animation = NONE_INDEX;
  gdr_index Light;
  gdr_index LightModel;
  ImGui::FileBrowser fileDialog;
  bool IsLastSuccess = true;
  std::string Asked = "";
public:
  void Initialize(void)
  {
    fileDialog.SetTitle("Choose a model...");
    fileDialog.SetTypeFilters({".obj", ".fbx", ".glb"});
    
    auto light_import_data = gdr::ImportModelFromAssimp("bin/models/light_meshes/cone.obj");
    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_load_any Init");
    LightModel = Engine->ModelsManager->Add(light_import_data);
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();

    Light = Engine->LightsSystem->Add();
    Engine->LightsSystem->GetEditable(Light).Color = mth::vec3f(10, 10, 10);
    Engine->LightsSystem->GetEditable(Light).LightSourceType = LIGHT_SOURCE_TYPE_SPOT;
    Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex = Engine->ModelsManager->Get(LightModel).Render.RootTransform;
    Engine->LightsSystem->GetEditable(Light).ConstantAttenuation = 1.0f;
    Engine->LightsSystem->GetEditable(Light).LinearAttenuation = 0.09f;
    Engine->LightsSystem->GetEditable(Light).QuadricAttenuation = 0.032f;
    Engine->LightsSystem->GetEditable(Light).AngleInnerCone = 45 * MTH_D2R;
    Engine->LightsSystem->GetEditable(Light).AngleOuterCone = 60 * MTH_D2R;
    Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex).Transform = mth::matr4f::RotateX(-30) * mth::matr4f::RotateY(-45) * mth::matr4f::Translate({10, 10, 0});
    Engine->ObjectTransformsSystem->IncreaseReferenceCount(Engine->ModelsManager->Get(LightModel).Render.RootTransform);
  }

  void LoadModel(std::string path)
  {
    if (Model != NONE_INDEX)
    {
      Engine->ModelsManager->Remove(Model);
      Engine->AnimationManager->Remove(Animation);
      Model = NONE_INDEX;
    }
    auto import_data = gdr::ImportModelFromAssimp(path);

    if (import_data.IsEmpty())
    {
      IsLastSuccess = false;
    }
    else
    {
      ID3D12GraphicsCommandList* commandList;
      Engine->GetDevice().BeginUploadCommandList(&commandList);
      PROFILE_BEGIN(commandList, charToWString(path.c_str()).c_str());
      Model = Engine->ModelsManager->Add(import_data);
      Animation = Engine->AnimationManager->Add(import_data);
      IsLastSuccess = true;
      PROFILE_END(commandList);
      Engine->GetDevice().CloseUploadCommandList();
    }
  }

  void Response(void)
  {
    if (Asked != "")
    {
      Engine->GetDevice().WaitAllUploadLists();
      Engine->GetDevice().ResizeUpdateBuffer(false);
      LoadModel(Asked);
      Engine->GetDevice().WaitAllUploadLists();
      Engine->GetDevice().ResizeUpdateBuffer(true);
      Asked = "";
    }
    Engine->AnimationManager->SetAnimationTime(Model, Animation, Engine->GetTime() * 1000);
    Engine->AddLambdaForIMGUI(
      [&]()
      {
        ImGui::Begin("Model open window");
          // open file dialog when user clicks this button
          if (ImGui::Button("open file dialog"))
            fileDialog.Open();
          if (!IsLastSuccess)
            ImGui::Text("Looks like your last attempt were unsuccessful");

          fileDialog.Display();

          if (fileDialog.HasSelected())
          {
            Asked = fileDialog.GetSelected().string();
            fileDialog.ClearSelected();
          }
        ImGui::End();
      });
  }

  std::string GetName(void)
  {
    return "unit_load_any";
  }

  ~unit_load_any(void)
  {
  }
};