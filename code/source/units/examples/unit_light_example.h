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
    auto import_data = gdr::ImportModelFromAssimp("bin/models/crazy_frog/crazy_frog.obj");
    auto light_import_data = gdr::ImportModelFromAssimp("bin/models/light_meshes/sphere.obj");
    
    import_data.Materials[0].ShadeType = MATERIAL_SHADER_PHONG;
    GDRGPUMaterialPhongGetAmbient(import_data.Materials[0]) = {0, 0, 0};
    GDRGPUMaterialPhongGetAmbientMapIndex(import_data.Materials[0]) = NONE_INDEX;
    GDRGPUMaterialPhongGetDiffuse(import_data.Materials[0]) = { 1, 1, 1};
    GDRGPUMaterialPhongGetDiffuseMapIndex(import_data.Materials[0]) = GDRGPUMaterialColorGetColorMapIndex(import_data.Materials[0]);
    GDRGPUMaterialPhongGetSpecular(import_data.Materials[0]) = { 0.3, 0.3, 0.3 };
    GDRGPUMaterialPhongGetSpecularMapIndex(import_data.Materials[0]) = NONE_INDEX;
    GDRGPUMaterialPhongGetShiness(import_data.Materials[0]) = 30;
    GDRGPUMaterialPhongGetNormalMapIndex(import_data.Materials[0]) = NONE_INDEX;

    // load Frog
    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_light_example Init Frog");
    Frog = Engine->ModelsManager->Add(import_data);
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();

    // create 3 lights
    for (int i = 0; i < LightAmount; i++)
      Lights.push_back(Engine->LightsSystem->Add());

    // load LightMeshes
    
    std::string OrigName = light_import_data.FileName;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_light_example Init lights");
    for (int i = 0; i < Lights.size(); i++)
    {
      light_import_data.Materials[0].ShadeType = MATERIAL_SHADER_COLOR;
      light_import_data.FileName = OrigName + std::to_string(i);
      GDRGPUMaterialColorGetColor(light_import_data.Materials[0]) = { 10.0f * rand() / RAND_MAX, 10.0f * rand() / RAND_MAX, 10.0f * rand() / RAND_MAX };
      GDRGPUMaterialColorGetColorMapIndex(light_import_data.Materials[0]) = NONE_INDEX;

      LightsModels.push_back(Engine->ModelsManager->Add(light_import_data));
    
      Engine->LightsSystem->GetEditable(Lights[i]).LightSourceType = LIGHT_SOURCE_TYPE_POINT;
      Engine->LightsSystem->GetEditable(Lights[i]).Color = GDRGPUMaterialColorGetColor(light_import_data.Materials[0]);
      Engine->LightsSystem->GetEditable(Lights[i]).ObjectTransformIndex = Engine->ModelsManager->Get(LightsModels[i]).Render.RootTransform;
      Engine->LightsSystem->GetEditable(Lights[i]).ConstantAttenuation = 1.0f;
      Engine->LightsSystem->GetEditable(Lights[i]).LinearAttenuation = 0.09f;
      Engine->LightsSystem->GetEditable(Lights[i]).QuadricAttenuation = 0.032f;
      Engine->ObjectTransformsSystem->IncreaseReferenceCount(Engine->ModelsManager->Get(LightsModels[i]).Render.RootTransform);
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
  }

  std::string GetName(void)
  {
    return "unit_light_example";
  }

  ~unit_light_example(void)
  {
  }
};