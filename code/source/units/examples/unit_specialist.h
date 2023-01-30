#pragma once
#include "../unit_base.h"

class unit_specialist : public gdr::unit_base
{
private:
  gdr::model_import_data SpecialistImportData;
  std::vector<gdr_index> Models;
  gdr_index Light;
  gdr_index LightModel;
  int DeltaSize = 0;
public:
  void Initialize(void)
  {
    SpecialistImportData = gdr::ImportModelFromAssimp("bin/models/specialist/specialist2.glb");

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
    Engine->LightsSystem->GetEditable(Light).AngleInnerCone = 45;
    Engine->LightsSystem->GetEditable(Light).AngleOuterCone = 60;
    Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex).Transform = mth::matr4f::RotateX(-30) * mth::matr4f::RotateY(45) * mth::matr4f::Translate({-10, 10, 0});
    Engine->ObjectTransformsSystem->IncreaseReferenceCount(Engine->ModelsManager->Get(LightModel).Render.RootTransform);
    DeltaSize = 50;
  }

  void PushBack(int amount = 1)
  {
    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "Load new element Init");
    for (int i = 0; i < amount; i++)
      Models.push_back(Engine->ModelsManager->Add(SpecialistImportData));
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();

    Engine->GetDevice().WaitAllUploadLists();
  }

  void PopBack(int amount = 1)
  {
    for (int i = 0; i < amount && Models.size() > 0; i++)
    {
      Engine->ModelsManager->Remove(Models[Models.size() - 1]);
      Models.pop_back();
    }
  }

  void Response(void)
  {
    Engine->AddLambdaForIMGUI(
      [&]()
      {
        ImGui::Begin("Specialist", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Current %d", Models.size());
        if (ImGui::Button("Add 1"))
          DeltaSize += 1;
        if (ImGui::Button("Remove 1"))
          DeltaSize -= 1;
        if (ImGui::Button("Add 10"))
          DeltaSize += 10;
        if (ImGui::Button("Remove 10"))
          DeltaSize -= 10;
        ImGui::End();
      });

    mth::matr rotation = mth::matr::RotateY(Engine->GetTime() * 50.0f);

    if (DeltaSize > 0)
      PushBack(DeltaSize);
    else if (DeltaSize < 0)
      PopBack(-DeltaSize);
    DeltaSize = 0;


    int row = max(sqrt(Models.size()), 2);
    /* single-thread computing */
    for (int i = 0; i < Models.size(); i++)
    {
      gdr_index ModelIndex = Models[i];
      gdr_index ModelRootTransform = Engine->ModelsManager->Get(ModelIndex).Render.RootTransform;
      float dist = (Engine->ObjectTransformsSystem->Get(ModelRootTransform).maxAABB - Engine->ObjectTransformsSystem->Get(ModelRootTransform).minAABB).Lenght();

      Engine->ObjectTransformsSystem->GetEditable(ModelRootTransform).Transform = rotation * mth::matr::Translate({ (i % row) * dist, 0, (i / row) * dist});

      Engine->ModelsManager->SetAnimationTime(ModelIndex, Engine->GetTime() * 1000.0);
    }
  }

  std::string GetName(void)
  {
    return "unit_specialist";
  }

  ~unit_specialist(void)
  {
  }
};