#pragma once
#include "../unit_base.h"

class unit_pbr_metallness : public gdr::unit_base
{
private:
    std::vector<gdr_index> Spheres;
    const int RoughnessStamps = 10;
    const int MetallnessStamps = 10;
    gdr_index Light;
public:
    void Initialize(void)
    {
        auto import_data = gdr::ImportMeshAssimp("bin/models/pbr_sphere/pbr_sphere.glb");

        import_data.Materials[0].ShadeType = MATERIAL_SHADER_COOKTORRANCE_METALNESS;
        GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(import_data.Materials[0]) = NONE_INDEX;
        GDRGPUMaterialCookTorranceGetAlbedo(import_data.Materials[0]) = mth::vec3f(1.0f, 0.71f, 0.29f);
        GDRGPUMaterialCookTorranceGetAlbedoMapIndex(import_data.Materials[0]) = NONE_INDEX;
        GDRGPUMaterialCookTorranceGetRoughnessMetallnessMapIndex(import_data.Materials[0]) = NONE_INDEX;
        GDRGPUMaterialCookTorranceGetNormalMapIndex(import_data.Materials[0]) = NONE_INDEX;


        ID3D12GraphicsCommandList* commandList;
        Engine->GetDevice().BeginUploadCommandList(&commandList);
        PROFILE_BEGIN(commandList, "unit_pbr_metallness Init");

        for (int i = 0; i < RoughnessStamps; i++)
            for (int j = 0; j < MetallnessStamps; j++)
            {
                float Roughness = 1.0f * i / (RoughnessStamps - 1);
                float Metallness = 1.0f * j / (MetallnessStamps - 1);
                GDRGPUMaterialCookTorranceGetRoughness(import_data.Materials[0]) = Roughness;
                GDRGPUMaterialCookTorranceGetMetallic(import_data.Materials[0]) = Metallness;
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
            for (int j = 0; j < MetallnessStamps; j++)
            {
                gdr_index ModelIndex = Spheres[i * MetallnessStamps + j];
                gdr_index TransformIndex = Engine->ModelsPool[ModelIndex].Rnd.RootTransform;
                Engine->ObjectTransformsSystem->GetEditable(TransformIndex).Transform = mth::matr::Translate({2.0f *  j, 2.0f * i, -10.0f});
            }
    }

    std::string GetName(void)
    {
        return "unit_pbr_metallness";
    }

    ~unit_pbr_metallness(void)
    {
    }
};