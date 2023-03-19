#pragma once
#include "../unit_base.h"

class unit_transparent : public gdr::unit_base
{
private:
  std::vector<gdr_index> Check;
  gdr_index LightMesh;
  gdr_index Light;
public:
  void Initialize(void)
  {
    auto import_data = gdr::ImportSplittedModelFromAssimp("bin/models/transparency_check/check.glb");
    auto light_import_data = gdr::ImportModelFromAssimp("bin/models/light_meshes/sphere.obj");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_normal_map Init");
    for (auto &data : import_data)
    {
      GDRGPUMaterialCookTorranceGetMetalness(data.Materials[0]) = 1.0;
      Check.push_back(Engine->ModelsManager->Add(data));
    }
    LightMesh = Engine->ModelsManager->Add(light_import_data);

    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();

    Light = Engine->LightsSystem->Add();
    Engine->LightsSystem->GetEditable(Light).Color = mth::vec3f(1, 1, 1);
    Engine->LightsSystem->GetEditable(Light).LightSourceType = LIGHT_SOURCE_TYPE_SPOT;
    Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex = Engine->ModelsManager->Get(LightMesh).Render.RootTransform;
    Engine->LightsSystem->GetEditable(Light).ConstantAttenuation = 1.0f;
    Engine->LightsSystem->GetEditable(Light).LinearAttenuation = 0.09f;
    Engine->LightsSystem->GetEditable(Light).QuadricAttenuation = 0.032f;
    Engine->LightsSystem->GetEditable(Light).ShadowMapIndex = Engine->ShadowMapsSystem->Add(2048, 2048);
    Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->GetEditable(Light).ObjectTransformIndex).Transform = mth::matr4f::Translate({ 20, 20, 0 });
    Engine->ObjectTransformsSystem->IncreaseReferenceCount(Engine->ModelsManager->Get(LightMesh).Render.RootTransform);
  }

  void Response(void)
  {
    gdr_index TransformIndex = Engine->ModelsManager->Get(LightMesh).Render.RootTransform;
    Engine->ObjectTransformsSystem->GetEditable(TransformIndex).Transform = mth::matr::Scale(0.1f) * mth::matr::Translate({ 0.f, 1.f, 1.f }) * mth::matr::RotateZ(Engine->GetTime() * 20);
  }

  std::string GetName(void)
  {
    return "unit_transparent";
  }

  ~unit_transparent(void)
  {
    for (auto& el : Check)
      Engine->ModelsManager->Remove(el);
    Engine->ModelsManager->Remove(LightMesh);
    Engine->LightsSystem->Remove(Light);
  }
};