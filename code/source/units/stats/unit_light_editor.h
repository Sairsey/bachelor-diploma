#pragma once
#include "../unit_base.h"

class unit_light_editor : public gdr::unit_base
{
private:
  int CurrentLightToShow;

  gdr_index PointLightObject;
  gdr_index DirLightObject;
  gdr_index SpotLightObject;
public:
  void Initialize()
  {
    CurrentLightToShow = 0;
    auto import_data_sphere = gdr::ImportModelFromAssimp("bin/models/light_meshes/sphere.obj");
    auto import_data_dir = gdr::ImportModelFromAssimp("bin/models/light_meshes/dir.obj");
    auto import_data_cone = gdr::ImportModelFromAssimp("bin/models/light_meshes/cone.obj");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_model_loading Init");
    PointLightObject = Engine->ModelsManager->Add(import_data_sphere);
    DirLightObject = Engine->ModelsManager->Add(import_data_dir);
    SpotLightObject = Engine->ModelsManager->Add(import_data_cone);
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }

  void Response(void)
  {
    Engine->ObjectTransformsSystem->GetEditable(
      Engine->ModelsManager->Get(PointLightObject).Render.RootTransform).Transform = mth::matr4f::Scale(0);
    Engine->ObjectTransformsSystem->GetEditable(
      Engine->ModelsManager->Get(DirLightObject).Render.RootTransform).Transform = mth::matr4f::Scale(0);
    Engine->ObjectTransformsSystem->GetEditable(
      Engine->ModelsManager->Get(SpotLightObject).Render.RootTransform).Transform = mth::matr4f::Scale(0);

    if (Engine->LightsSystem->IsExist(CurrentLightToShow) && Engine->LightsSystem->Get(CurrentLightToShow).ObjectTransformIndex != NONE_INDEX)
    {
      // choose right mesh
      gdr_index ObjectType;
      if (Engine->LightsSystem->Get(CurrentLightToShow).LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
        ObjectType = DirLightObject;
      if (Engine->LightsSystem->Get(CurrentLightToShow).LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
        ObjectType = SpotLightObject;
      if (Engine->LightsSystem->Get(CurrentLightToShow).LightSourceType == LIGHT_SOURCE_TYPE_POINT)
        ObjectType = PointLightObject;

      Engine->ObjectTransformsSystem->GetEditable(
        Engine->ModelsManager->Get(ObjectType).Render.RootTransform).Transform =
        Engine->ObjectTransformsSystem->Get(Engine->LightsSystem->Get(CurrentLightToShow).ObjectTransformIndex).Transform;
      if (ObjectType == DirLightObject)
      {
        Engine->ObjectTransformsSystem->GetEditable(
          Engine->ModelsManager->Get(ObjectType).Render.RootTransform).Transform *= mth::matr::Translate({ 0, 10, 0 });
      }
    }

    Engine->AddLambdaForIMGUI(
      [&]()
      {
        if (Engine->LightsSystem->AllocatedSize() == 0)
          return;
        ImGui::Begin("Lights Viewer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("<<<"))
        {
          CurrentLightToShow -= 100;
        }
        ImGui::SameLine();
        if (ImGui::Button("<<"))
        {
          CurrentLightToShow -= 10;
        }
        ImGui::SameLine();
        if (ImGui::Button("<"))
        {
          CurrentLightToShow -= 1;
        }
        ImGui::SameLine();
        if (ImGui::Button(">"))
        {
          CurrentLightToShow += 1;
        }
        ImGui::SameLine();
        if (ImGui::Button(">>"))
        {
          CurrentLightToShow += 10;
        }
        ImGui::SameLine();
        if (ImGui::Button(">>>"))
        {
          CurrentLightToShow += 100;
        }

        CurrentLightToShow = min(max(0, CurrentLightToShow), Engine->LightsSystem->AllocatedSize() - 1);

        if (ImGui::Button("New Light"))
        {
          CurrentLightToShow = Engine->LightsSystem->Add();
          Engine->LightsSystem->GetEditable(CurrentLightToShow).ObjectTransformIndex = Engine->ObjectTransformsSystem->Add();
          ImGui::End();
          return;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete Light"))
        {
          Engine->LightsSystem->Remove(CurrentLightToShow);
          ImGui::End();
          return;
        }


        ImGui::SameLine();

        ImGui::Text("Light %d", CurrentLightToShow);

        if (Engine->LightsSystem->IsExist(CurrentLightToShow))
        {
          const char* items[] = { "Directional" , "Point", "Spot" };
          ImGui::Combo("Type", (int*)&Engine->LightsSystem->Get(CurrentLightToShow).LightSourceType, items, IM_ARRAYSIZE(items));

          ImGui::Text("Transform index %d", Engine->LightsSystem->Get(CurrentLightToShow).ObjectTransformIndex);
          
          if (Engine->LightsSystem->Get(CurrentLightToShow).ObjectTransformIndex != NONE_INDEX)
          {
            mth::vec3f Translate, Rotate, Scale;
            Engine->ObjectTransformsSystem->Get(Engine->LightsSystem->Get(CurrentLightToShow).ObjectTransformIndex).Transform.Decompose(Translate, Rotate, Scale);

            bool IsMatrChanged = false;

            if (ImGui::DragFloat3("Translation", &Translate.X, 0.1))
              IsMatrChanged = true;
            if (ImGui::DragFloat3("Rotation", &Rotate.X, 0.1))
              IsMatrChanged = true;
            if (ImGui::DragFloat3("Scale", &Scale.X, 0.1))
              IsMatrChanged = true;

            if (IsMatrChanged)
            {
              mth::matr4f result = mth::matr4f::RotateZ(Rotate.Z) * mth::matr4f::RotateY(Rotate.Y) * mth::matr4f::RotateX(Rotate.X) * mth::matr4f::Scale(Scale) * mth::matr4f::Translate(Translate);
              if (!isnan(result[0][0]) && !isnan(result[1][1]) && !isnan(result[2][2]) && !isnan(result[3][3]))
                Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->Get(CurrentLightToShow).ObjectTransformIndex).Transform = result;
              else
                Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->Get(CurrentLightToShow).ObjectTransformIndex).Transform= mth::matr4f::Identity();
            }
          }

          ImGui::ColorEdit3("Color", &Engine->LightsSystem->GetEditable(CurrentLightToShow).Color[0]);
          ImGui::DragFloat("Red", &Engine->LightsSystem->GetEditable(CurrentLightToShow).Color[0], 0.1);
          ImGui::DragFloat("Green", &Engine->LightsSystem->GetEditable(CurrentLightToShow).Color[1], 0.1);
          ImGui::DragFloat("Blue", &Engine->LightsSystem->GetEditable(CurrentLightToShow).Color[2], 0.1);

          if (Engine->LightsSystem->Get(CurrentLightToShow).LightSourceType != LIGHT_SOURCE_TYPE_DIRECTIONAL)
          {
            ImGui::Text("Attenuation");
            ImGui::DragFloat("Constant part", &Engine->LightsSystem->GetEditable(CurrentLightToShow).ConstantAttenuation, 0.1);
            ImGui::DragFloat("Linear part", &Engine->LightsSystem->GetEditable(CurrentLightToShow).LinearAttenuation, 0.1);
            ImGui::DragFloat("Quadric part", &Engine->LightsSystem->GetEditable(CurrentLightToShow).QuadricAttenuation, 0.1);
            Engine->LightsSystem->GetEditable(CurrentLightToShow).ConstantAttenuation = 
              max(1, Engine->LightsSystem->Get(CurrentLightToShow).ConstantAttenuation);
          }

          if (Engine->LightsSystem->Get(CurrentLightToShow).LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
          {
            Engine->LightsSystem->GetEditable(CurrentLightToShow).AngleInnerCone *= MTH_R2D;
            Engine->LightsSystem->GetEditable(CurrentLightToShow).AngleOuterCone *= MTH_R2D;
            ImGui::DragFloat("Inner cone angle", &Engine->LightsSystem->GetEditable(CurrentLightToShow).AngleInnerCone, 0.1, 1);
            ImGui::DragFloat("Outer cone angle", &Engine->LightsSystem->GetEditable(CurrentLightToShow).AngleOuterCone, 0.1, Engine->LightsSystem->GetEditable(CurrentLightToShow).AngleInnerCone);
            Engine->LightsSystem->GetEditable(CurrentLightToShow).AngleInnerCone = max(5, Engine->LightsSystem->Get(CurrentLightToShow).AngleInnerCone);
            Engine->LightsSystem->GetEditable(CurrentLightToShow).AngleOuterCone = max(Engine->LightsSystem->Get(CurrentLightToShow).AngleInnerCone, Engine->LightsSystem->Get(CurrentLightToShow).AngleOuterCone);
            Engine->LightsSystem->GetEditable(CurrentLightToShow).AngleInnerCone *= MTH_D2R;
            Engine->LightsSystem->GetEditable(CurrentLightToShow).AngleOuterCone *= MTH_D2R;

            if (Engine->LightsSystem->Get(CurrentLightToShow).ShadowMapIndex == NONE_INDEX)
            {
              static int W = 128, H = 128;
              ImGui::Text("Shadow map");
              ImGui::SameLine();
              ImGui::DragInt("W", &W);
              ImGui::SameLine();
              ImGui::DragInt("H", &H);
              ImGui::SameLine();
              if (ImGui::Button("Add"))
                Engine->LightsSystem->GetEditable(CurrentLightToShow).ShadowMapIndex = Engine->ShadowMapsSystem->Add(W, H);
            }
            else
            {
              int ShadowMapIndex = Engine->LightsSystem->Get(CurrentLightToShow).ShadowMapIndex;
              auto& el = Engine->ShadowMapsSystem->Get(ShadowMapIndex);

              ImGui::Text("Shadow map index %d", ShadowMapIndex); 
              ImGui::SameLine();
              if (ImGui::Button("Delete"))
              {
                Engine->ShadowMapsSystem->Remove(ShadowMapIndex);
                Engine->LightsSystem->GetEditable(CurrentLightToShow).ShadowMapIndex = NONE_INDEX;
              }
              else
              {
                ImGui::Text("Shadow map");
                ImGui::Text("Width : %d", el.W);
                ImGui::Text("Heigth: %d", el.H);
                ImGui::DragFloat("Offset", &Engine->LightsSystem->GetEditable(CurrentLightToShow).ShadowMapOffset);

                D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->ShadowMapsSystem->ShadowMapTableGPU;
                true_texture_handle.ptr += ShadowMapIndex * Engine->GetDevice().GetSRVDescSize();
                ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
              }
            }

          }
        }
        else
        {
          ImGui::Text("Not Exist");
        }
        ImGui::End();
      });
  }
};
