#pragma once
#include "unit_base.h"

class unit_pit : public gdr::unit_base
{
private:
    gdr::gdr_index PitRender;
    gdr::gdr_index PitPhys;

    std::vector<gdr::gdr_index> SpheresRender;
    std::vector<gdr::gdr_index> SpheresPhysic;

    int SphereAmount = 0;

    bool CreateSpheres = false;

public:

    unit_pit(int amount = 10)
    {
        SphereAmount = amount;
    }

    void Initialize(void)
    {
        PitRender = Engine->ObjectSystem->CreateObjectFromFile("bin/models/pit/pit.glb");
        PitPhys = Engine->NewStaticMeshAssimp(gdr::physic_material(), "bin/models/pit/pit.glb")[0];
        SpheresRender.push_back(Engine->ObjectSystem->CreateObjectFromFile("bin/models/pbr_sphere/pbr_sphere.glb"));
        SpheresPhysic.push_back(Engine->NewDynamicSphere(gdr::physic_material(), 1, "sphere"));
        Engine->GetPhysObject(SpheresPhysic[SpheresPhysic.size() - 1]).SetPos(mth::vec3f(2.0 * rand() / RAND_MAX - 1, 20, 2.0 * rand() / RAND_MAX - 1));
        Engine->GetPhysObject(SpheresPhysic[SpheresPhysic.size() - 1]).AddVelocity({ 0, -20, 0 });
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
                SpheresRender.push_back(Engine->ObjectSystem->CreateObjectFromFile("bin/models/pbr_sphere/pbr_sphere.glb"));
                SpheresPhysic.push_back(Engine->NewDynamicSphere(gdr::physic_material(), 1, "sphere"));
                Engine->GetPhysObject(SpheresPhysic[SpheresPhysic.size() - 1]).SetPos(mth::vec3f(2.0 * rand() / RAND_MAX - 1, 2 * float(i) + 20, 2.0 * rand() / RAND_MAX - 1));
                Engine->GetPhysObject(SpheresPhysic[SpheresPhysic.size() - 1]).AddVelocity({ 0, -20, 0 });
            }
        }

        // update each sphere according to its transform
        for (int i = 0; i < SpheresRender.size(); i++)
        {
            Engine->ObjectSystem->NodesPool[SpheresRender[i]].GetTransformEditable() = Engine->GetPhysObject(SpheresPhysic[i]).GetTransform();
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