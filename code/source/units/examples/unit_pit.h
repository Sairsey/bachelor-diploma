#pragma once
#include "../unit_base.h"

class unit_pit : public gdr::unit_base
{
private:
    gdr::model_import_data PitImportData;
    gdr_index PitModel;
    gdr_index PitPhys;

    gdr::model_import_data SphereImportData;
    std::vector<gdr_index> SpheresModels;
    std::vector<gdr_index> SpheresPhysic;

    int SphereAmount = 0;

    bool CreateSpheres = false;

public:

    unit_pit(int amount = 10)
    {
        SphereAmount = amount;
    }

    void Initialize(void)
    {
      PitImportData = gdr::ImportModelFromAssimp("bin/models/pit/pit.glb");
      SphereImportData = gdr::ImportModelFromAssimp("bin/models/pbr_sphere/pbr_sphere.glb");

      ID3D12GraphicsCommandList* commandList;
      Engine->GetDevice().BeginUploadCommandList(&commandList);
      {
        PROFILE_BEGIN(commandList, "unit_pit Init pit geometry");
        PitModel = Engine->ModelsManager->Add(PitImportData);
        PROFILE_END(commandList);
      }
      Engine->GetDevice().CloseUploadCommandList();

      Engine->PhysicsManager->AddStaticMesh(PitImportData);
      Engine->GetDevice().BeginUploadCommandList(&commandList);
      {
        PROFILE_BEGIN(commandList, "unit_pit Init sphere geometry");
        SpheresModels.push_back(Engine->ModelsManager->Add(SphereImportData));
        PROFILE_END(commandList);
      }
      Engine->GetDevice().CloseUploadCommandList();
      SpheresPhysic.push_back(Engine->PhysicsManager->AddDynamicSphere(1.0f));
      Engine->PhysicsManager->GetEditable(SpheresPhysic[SpheresPhysic.size() - 1]).SetPos({ 2.0f * rand() / RAND_MAX - 1.f, 20.f, 2.0f * rand() / RAND_MAX - 1.f });
      Engine->PhysicsManager->GetEditable(SpheresPhysic[SpheresPhysic.size() - 1]).SetParent(SpheresModels[SpheresModels.size() - 1]);

    }

    void Response(void)
    {
        Engine->AddLambdaForIMGUI([&]() {
            ImGui::Begin("Simulation", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::DragInt("Spheres Amount", &SphereAmount);
            if (ImGui::Button("Start"))
            {
                CreateSpheres = true;
            }
            ImGui::End();
            });


        if (CreateSpheres)
        {
            CreateSpheres = false;
            for (int i = 0; i < SphereAmount; i++)
            {
              ID3D12GraphicsCommandList* commandList;
              Engine->GetDevice().BeginUploadCommandList(&commandList);
              {
                PROFILE_BEGIN(commandList, "unit_pit Init sphere geometry");
                SpheresModels.push_back(Engine->ModelsManager->Add(SphereImportData));
                PROFILE_END(commandList);
              }
              Engine->GetDevice().CloseUploadCommandList();
              SpheresPhysic.push_back(Engine->PhysicsManager->AddDynamicSphere(1.0f));
              Engine->PhysicsManager->GetEditable(SpheresPhysic[SpheresPhysic.size() - 1]).SetPos({ 2.0f * rand() / RAND_MAX - 1.f, 2.f * float(i) + 20.f, 2.0f * rand() / RAND_MAX - 1.f });
              Engine->PhysicsManager->GetEditable(SpheresPhysic[SpheresPhysic.size() - 1]).AddVelocity({ 0, -20, 0 });
              Engine->PhysicsManager->GetEditable(SpheresPhysic[SpheresPhysic.size() - 1]).SetParent(SpheresModels[SpheresModels.size() - 1]);
            }
        }

        // update each sphere according to its transform
        for (int i = 0; i < SpheresPhysic.size(); i++)
        {
            gdr_index ModelIndex = SpheresModels[i];
            gdr_index PhysicIndex = SpheresPhysic[i];
            gdr_index ModelTransformIndex = Engine->ModelsManager->Get(ModelIndex).Render.RootTransform;
            Engine->ObjectTransformsSystem->GetEditable(ModelTransformIndex).Transform = Engine->PhysicsManager->Get(PhysicIndex).GetTransform();
        }
    }

    std::string GetName(void)
    {
        return "unit_pit";
    }

    ~unit_pit(void)
    {
    }
};