#pragma once
#include "../unit_base.h"

class unit_runtime_add_remove : public gdr::unit_base
{
private:
  // model import data
  gdr::mesh_import_data import_data;
  std::vector<gdr_index> Models;
  int DeltaSize = 0;
public:
  void Initialize(void)
  {
    import_data = gdr::ImportMeshAssimp("bin/models/bread/bread.obj");
    //Engine->EnableFullscreen();
  }

  void PushBack(int amount = 1)
  {
    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "Load new element Init");
    for (int i = 0; i < amount; i++)
      Models.push_back(Engine->AddModel(import_data));
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();

    Engine->GetDevice().WaitAllUploadLists();
  }

  void PopBack(int amount = 1)
  {
    for (int i = 0; i < amount && Models.size() > 0; i++)
    {
      Engine->DeleteModel(Models[Models.size() - 1]);
      Models.pop_back();
    }
  }

  void Response(void)
  {
    Engine->AddLambdaForIMGUI(
      [&]()
      {
        ImGui::Begin("Runtime Add/Remove", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("Add 1"))
          DeltaSize += 1;
        if (ImGui::Button("Remove 1"))
          DeltaSize -= 1;
        if (ImGui::Button("Add 10"))
          DeltaSize += 10;
        if (ImGui::Button("Remove 10"))
          DeltaSize -= 10;
        if (ImGui::Button("Add 100"))
          DeltaSize += 100;
        if (ImGui::Button("Remove 100"))
          DeltaSize -= 100;
        ImGui::End();
      });

    mth::matr rotation = mth::matr::RotateY(Engine->GetTime() * 50.0f);

    if (DeltaSize > 0)
      PushBack(DeltaSize);
    else if (DeltaSize < 0)
      PopBack(-DeltaSize);
    DeltaSize = 0;

    /* single-thread computing */
    float alpha = 0;
    for (int i = 0; i < Models.size(); i++)
    {
      gdr_index ModelIndex = Models[i];
      gdr_index ModelRootTransform = Engine->ModelsPool[ModelIndex].Rnd.RootTransform;
      float dist = (Engine->ObjectTransformsSystem->Get(ModelRootTransform).maxAABB - Engine->ObjectTransformsSystem->Get(ModelRootTransform).minAABB).Lenght();
      alpha = sqrt(alpha * alpha + dist);
      float radius = alpha * 2;

      Engine->ObjectTransformsSystem->GetEditable(ModelRootTransform).Transform = rotation * mth::matr::Translate({ sin(alpha) * radius, 0, cos(alpha) * radius });
    }
  }

  std::string GetName(void)
  {
    return "unit_runtime_add_remove";
  }

  ~unit_runtime_add_remove(void)
  {
  }
};