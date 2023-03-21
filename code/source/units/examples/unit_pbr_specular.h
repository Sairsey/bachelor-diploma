#pragma once
#include "../unit_base.h"

class unit_pbr_specular : public gdr::unit_base
{
private:
    gdr_index Fiat;
    gdr_index Light;
public:
    void Initialize(void)
    {
        auto import_data = gdr::ImportModelFromAssimp("bin/models/fiat/fiat.glb");
        //auto import_data = gdr::ImportMeshAssimp("bin/models/sketchbook/sketchbook.glb");

        ID3D12GraphicsCommandList* commandList;
        Engine->GetDevice().BeginUploadCommandList(&commandList);
        PROFILE_BEGIN(commandList, "unit_pbr_specular Init");

        Fiat = Engine->ModelsManager->Add(import_data);

        PROFILE_END(commandList);
        Engine->GetDevice().CloseUploadCommandList();

        Light = Engine->LightsSystem->Add();
        Engine->LightsSystem->GetEditable(Light).Color = mth::vec3f(5, 5, 5);
        Engine->LightsSystem->GetEditable(Light).LightSourceType = LIGHT_SOURCE_TYPE_POINT;
        Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex = Engine->ObjectTransformsSystem->Add();
        Engine->LightsSystem->GetEditable(Light).ConstantAttenuation = 1.0f;
        Engine->LightsSystem->GetEditable(Light).LinearAttenuation = 0.09f;
        Engine->LightsSystem->GetEditable(Light).QuadricAttenuation = 0.032f;
        Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex).Transform = mth::matr4f::Translate({ 5, 5, 0 });
        Engine->ObjectTransformsSystem->IncreaseReferenceCount(Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex);
    }

    void Response(void)
    {
        gdr_index TransformIndex = Engine->ModelsManager->Get(Fiat).Render.RootTransform;
        Engine->ObjectTransformsSystem->GetEditable(TransformIndex).Transform = mth::matr::RotateY(Engine->GetTime());
    }

    std::string GetName(void)
    {
        return "unit_pbr_specular";
    }

    ~unit_pbr_specular(void)
    {
      Engine->ModelsManager->Remove(Fiat);
      Engine->LightsSystem->Remove(Light);
    }
};