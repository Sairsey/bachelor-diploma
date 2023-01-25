#pragma once
#include "../unit_base.h"

class unit_light_example : public gdr::unit_base
{
private:
  gdr_index Frog;
  const int LightAmount = 10;
  std::vector<gdr_index> Lights;
  std::vector<gdr_index> LightsModels;
public:
  void Initialize(void)
  {
    auto import_data = gdr::ImportMeshAssimp("bin/models/sketchbook/sketchbook.glb");
    auto light_import_data = gdr::ImportMeshAssimp("bin/models/light_meshes/sphere.obj");
    
    // load Frog
    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_light_example Init Frog");
    Frog = Engine->AddModel(import_data);
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();

    // create 3 lights
    for (int i = 0; i < LightAmount; i++)
      Lights.push_back(Engine->LightsSystem->Add());

    // load LightMeshes
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_light_example Init lights");
    for (int i = 0; i < Lights.size(); i++)
    {
      light_import_data.Materials[0].ShadeType = MATERIAL_SHADER_COLOR;

      GDRGPUMaterialColorGetColor(light_import_data.Materials[0]) = { 1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX };
      GDRGPUMaterialColorGetColorMapIndex(light_import_data.Materials[0]) = NONE_INDEX;

      LightsModels.push_back(Engine->AddModel(light_import_data));
    
      Engine->LightsSystem->GetEditable(Lights[i]).LightSourceType = LIGHT_SOURCE_TYPE_POINT;
      Engine->LightsSystem->GetEditable(Lights[i]).Color = GDRGPUMaterialColorGetColor(light_import_data.Materials[0]);
      Engine->LightsSystem->GetEditable(Lights[i]).ObjectTransformIndex = Engine->ModelsPool[LightsModels[i]].Rnd.RootTransform;
      Engine->LightsSystem->GetEditable(Lights[i]).ConstantAttenuation = 1.0f;
      Engine->LightsSystem->GetEditable(Lights[i]).LinearAttenuation = 0.09f;
      Engine->LightsSystem->GetEditable(Lights[i]).QuadricAttenuation = 0.032f;
    }
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }

  void Response(void)
  {
    float radius = 1;
    float alpha_step = 360.0f / (Lights.size());
    mth::matr translation = mth::matr::Scale({0.1f}) * mth::matr::Translate(mth::vec3f(radius, 0, 0));
    for (int i = 0; i < Lights.size(); i++)
      Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->Get(Lights[i]).ObjectTransformIndex).Transform 
        = translation * mth::matr::RotateY(alpha_step * i + Engine->GetTime() * 360.0f / 10.0f);
    Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsPool[Frog].Rnd.RootTransform).Transform = mth::matr::Translate({ 0, -5, 0 });
  }

  std::string GetName(void)
  {
    return "unit_light_example";
  }

  ~unit_light_example(void)
  {
  }
};