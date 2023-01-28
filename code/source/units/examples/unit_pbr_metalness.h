#pragma once
#include "../unit_base.h"

class unit_pbr_metalness : public gdr::unit_base
{
private:
    std::vector<gdr_index> Spheres;
    const int RoughnessStamps = 10;
    const int MetalnessStamps = 10;
    gdr_index Light;
public:
    void Initialize(void)
    {
        auto import_data = gdr::ImportMeshAssimp("bin/models/pbr_sphere/pbr_sphere.glb");

        ID3D12GraphicsCommandList* commandList;
        Engine->GetDevice().BeginUploadCommandList(&commandList);
        PROFILE_BEGIN(commandList, "unit_pbr_metalness Init");
        std::string OriginalName = import_data.FileName;
        for (int i = 0; i < RoughnessStamps; i++)
            for (int j = 0; j < MetalnessStamps; j++)
            {
                float Roughness = 1.0f * i / (RoughnessStamps - 1);
                float Metalness = 1.0f * j / (MetalnessStamps - 1);
                import_data.FileName = OriginalName + std::to_string(i) + "_" + std::to_string(j);
                GDRGPUMaterialCookTorranceGetRoughness(import_data.Materials[0]) = Roughness;
                GDRGPUMaterialCookTorranceGetMetalness(import_data.Materials[0]) = Metalness;
                Spheres.push_back(Engine->AddModel(import_data));
            }

        PROFILE_END(commandList);
        Engine->GetDevice().CloseUploadCommandList();

        Light = Engine->LightsSystem->Add();
        Engine->LightsSystem->GetEditable(Light).Color = mth::vec3f(5, 5, 5);
        Engine->LightsSystem->GetEditable(Light).LightSourceType = LIGHT_SOURCE_TYPE_POINT;
        Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex = Engine->ObjectTransformsSystem->Add();
        Engine->LightsSystem->GetEditable(Light).ConstantAttenuation = 1.0f;
        Engine->LightsSystem->GetEditable(Light).LinearAttenuation = 0.09f;
        Engine->LightsSystem->GetEditable(Light).QuadricAttenuation = 0.032f;
        Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex).Transform = mth::matr4f::Translate({20, 20, 0});
    }

    void Response(void)
    {
        for (int i = 0; i < RoughnessStamps; i++)
            for (int j = 0; j < MetalnessStamps; j++)
            {
                gdr_index ModelIndex = Spheres[i * MetalnessStamps + j];
                gdr_index TransformIndex = Engine->ModelsPool[ModelIndex].Rnd.RootTransform;
                Engine->ObjectTransformsSystem->GetEditable(TransformIndex).Transform = mth::matr::Translate({2.0f *  j, 2.0f * i, -10.0f});
            }
    }

    std::string GetName(void)
    {
        return "unit_pbr_metalness";
    }

    ~unit_pbr_metalness(void)
    {
    }
};