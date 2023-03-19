#pragma once
#include "unit_editor.h"
#include "../unit_base.h"

class unit_editor_window_edit : public gdr::unit_base
{
public:
  void Initialize(void)
  {
    FloatVars["Show"] = 1;
    auto import_data_sphere = gdr::ImportModelFromAssimp("bin/models/light_meshes/sphere.obj");
    auto import_data_dir = gdr::ImportModelFromAssimp("bin/models/light_meshes/dir.obj");
    auto import_data_cone = gdr::ImportModelFromAssimp("bin/models/light_meshes/cone.obj");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "load light objects");
    IndicesVars["PointMesh"] = Engine->ModelsManager->Add(import_data_sphere);
    IndicesVars["DirMesh"] = Engine->ModelsManager->Add(import_data_dir);
    IndicesVars["ConeMesh"] = Engine->ModelsManager->Add(import_data_cone);
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();

    gdr_index UtilityMeshes[] = { IndicesVars["PointMesh"], IndicesVars["DirMesh"], IndicesVars["ConeMesh"] };

    for (int i = 0; i < _countof(UtilityMeshes); i++)
    {
      auto meshes = Engine->ModelsManager->Get(UtilityMeshes[i]).Render.Meshes;
      for (int j = 0; j < meshes.size(); j++)
      {
        Engine->DrawCommandsSystem->GetEditable(Engine->ModelsManager->Get(UtilityMeshes[i]).Render.Hierarchy[meshes[j]].DrawCommand).Indices.ObjectParamsMask |= OBJECT_PARAMETER_TOP_MOST;
      }
    }
  }

  void ShowEditorCamera()
  {
    ImGui::Text("Camera Editor");
    ImGui::DragFloat3("Camera Position", const_cast<float*>(&Engine->PlayerCamera.GetPos().X), 0.1f);
    ImGui::DragFloat3("Camera Direction", const_cast<float*>(&Engine->PlayerCamera.GetDir().X), 0.1f);
    ImGui::DragFloat3("Camera Up", const_cast<float*>(&Engine->PlayerCamera.GetUp().X), 0.1f);
    if (ImGui::Button("Apply Camera Transform"))
      Engine->PlayerCamera.SetView(Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetDir() + Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetUp());
  }

  void ShowEditorUnit()
  {
    ImGui::Text("Unit Editor");

    /*
    std::unordered_map<std::string, gdr_index> IndicesVars;
    std::unordered_map<std::string, float> FloatVars;
    std::unordered_map<std::string, float2> Float2Vars;
    std::unordered_map<std::string, float3> Float3Vars;
    std::unordered_map<std::string, float4> Float4Vars;
    std::unordered_map<std::string, mth::matr4f> MatrVars;
    std::unordered_map<std::string, std::string> StringVars;
    
    ImGui::BeginTable("Indices", 4, ImGuiTableFlags_Borders);
    ImGui::TableNextColumn();
    ImGui::Text("Key");
    ImGui::TableNextColumn();
    ImGui::Text("Type");
    ImGui::TableNextColumn();
    ImGui::Text("Value");
    ImGui::TableNextColumn();
    ImGui::Text("Goto");
    ImGui::TableNextRow();
    ImGui::EndTable();
    for (int i = 0; i < Engine->SceneUnit->Get(ChoosedElement).Render.Materials.size(); i++)
    {
      ImGui::TableNextColumn();
      ImGui::Text("%d", Engine->ModelsManager->Get(ChoosedElement).Render.Materials[i].value);
      ImGui::TableNextColumn();
      if (Engine->ModelsManager->Get(ChoosedElement).Render.Materials[i].value != NONE_INDEX)
      {
        if (ImGui::Button((std::string("Go to material ") + std::to_string(Engine->ModelsManager->Get(ChoosedElement).Render.Materials[i].value)).c_str()))
        {
          ChoosedElement = Engine->ModelsManager->Get(ChoosedElement).Render.Materials[i];
          ChoosedElement.type = gdr_index_types::material;
          ImGui::TableNextRow();
          ImGui::EndTable();
          return;
        }
      }
      ImGui::TableNextRow();
    }
    */
  }

  void ShowEditResourceLight(void)
  {
    ImGui::Text("Light Editor");
    if (Engine->LightsSystem->IsExist(ParentUnit->IndicesVars["ChoosedElement"]))
    {
      if (ImGui::Button("Delete"))
      {
        ParentUnit->FloatVars["Remove"] = true;
        return;
      }
      const char* items[] = { "Directional" , "Point", "Spot" };
      ImGui::Combo("Type", (int*)&Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).LightSourceType, items, IM_ARRAYSIZE(items));
      ImGui::Text("Transform index %d", Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ObjectTransformIndex);

      if (Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ObjectTransformIndex != NONE_INDEX)
      {
        mth::vec3f Translate, Rotate, Scale;
        Engine->ObjectTransformsSystem->Get(Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ObjectTransformIndex).Transform.Decompose(Translate, Rotate, Scale);

        bool IsMatrChanged = false;

        if (ImGui::DragFloat3("Translation", &Translate.X, 0.1f))
          IsMatrChanged = true;
        if (ImGui::DragFloat3("Rotation", &Rotate.X, 0.1f))
          IsMatrChanged = true;
        if (ImGui::DragFloat3("Scale", &Scale.X, 0.1f))
          IsMatrChanged = true;

        if (IsMatrChanged)
        {
          mth::matr4f result = mth::matr4f::RotateZ(Rotate.Z) * mth::matr4f::RotateY(Rotate.Y) * mth::matr4f::RotateX(Rotate.X) * mth::matr4f::Scale(Scale) * mth::matr4f::Translate(Translate);
          if (!isnan(result[0][0]) && !isnan(result[1][1]) && !isnan(result[2][2]) && !isnan(result[3][3]))
            Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ObjectTransformIndex).Transform = result;
          else
            Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ObjectTransformIndex).Transform = mth::matr4f::Identity();
        }
      }
      else if (ImGui::Button("Add"))
        Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).ObjectTransformIndex = Engine->ObjectTransformsSystem->Add();

      ImGui::ColorEdit3("Color", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).Color[0]);
      ImGui::DragFloat("Red", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).Color[0], 0.1f);
      ImGui::DragFloat("Green", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).Color[1], 0.1f);
      ImGui::DragFloat("Blue", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).Color[2], 0.1f);

      if (Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).LightSourceType != LIGHT_SOURCE_TYPE_DIRECTIONAL)
      {
        ImGui::Text("Attenuation");
        ImGui::DragFloat("Constant part", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).ConstantAttenuation, 0.1f);
        ImGui::DragFloat("Linear part", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).LinearAttenuation, 0.1f);
        ImGui::DragFloat("Quadric part", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).QuadricAttenuation, 0.1f);
        Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).ConstantAttenuation =
          max(1, Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ConstantAttenuation);
      }

      if (Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
      {
        Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleInnerCone *= MTH_R2D;
        Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleOuterCone *= MTH_R2D;
        ImGui::DragFloat("Inner cone angle", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleInnerCone, 0.1f, 1);
        ImGui::DragFloat("Outer cone angle", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleOuterCone, 0.1f, Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleInnerCone);
        Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleInnerCone = max(5, Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).AngleInnerCone);
        Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleOuterCone = max(Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).AngleInnerCone, Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).AngleOuterCone);
        Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleInnerCone *= MTH_D2R;
        Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleOuterCone *= MTH_D2R;
      }
      else if (Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
      {
        ImGui::DragFloat("Shadow Map Size", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).AngleInnerCone, 0.1f, 1);
      }

      if (Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).LightSourceType != LIGHT_SOURCE_TYPE_POINT)
      {
        if (Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ShadowMapIndex == NONE_INDEX)
        {
          static int W = 128, H = 128;
          ImGui::Text("Shadow map");
          ImGui::DragInt("W", &W);
          ImGui::DragInt("H", &H);
          if (ImGui::Button("Add"))
            Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).ShadowMapIndex = Engine->ShadowMapsSystem->Add(W, H);
        }
        else
        {
          int ShadowMapIndex = Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ShadowMapIndex;
          auto& el = Engine->ShadowMapsSystem->Get(ShadowMapIndex);

          ImGui::Text("Shadow map index %d", ShadowMapIndex);
          if (ImGui::Button("Delete shadow map"))
          {
            Engine->ShadowMapsSystem->Remove(ShadowMapIndex);
            Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).ShadowMapIndex = NONE_INDEX;
          }

          ImGui::Text("Width : %d", el.W);
          ImGui::Text("Heigth: %d", el.H);
          ImGui::DragFloat("Offset", &Engine->LightsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).ShadowMapOffset);

          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->ShadowMapsSystem->ShadowMapTableGPU;
          true_texture_handle.ptr += ShadowMapIndex * Engine->GetDevice().GetSRVDescSize();
          ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
        }
      }
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceObjectTransform(void)
  {
    ImGui::Text("Object Transform Editor");
    if (Engine->ObjectTransformsSystem->IsExist(ParentUnit->IndicesVars["ChoosedElement"]))
    {
      mth::vec3f Translate, Rotate, Scale;
      Engine->ObjectTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).Transform.Decompose(Translate, Rotate, Scale);

      bool IsMatrChanged = false;

      if (ImGui::DragFloat3("Translation", &Translate.X, 0.1f))
        IsMatrChanged = true;
      if (ImGui::DragFloat3("Rotation", &Rotate.X, 0.1f))
        IsMatrChanged = true;
      if (ImGui::DragFloat3("Scale", &Scale.X, 0.1f))
        IsMatrChanged = true;

      if (IsMatrChanged)
      {
        mth::matr4f result = mth::matr4f::RotateZ(Rotate.Z) * mth::matr4f::RotateY(Rotate.Y) * mth::matr4f::RotateX(Rotate.X) * mth::matr4f::Scale(Scale) * mth::matr4f::Translate(Translate);
        if (!isnan(result[0][0]) && !isnan(result[1][1]) && !isnan(result[2][2]) && !isnan(result[3][3]))
          Engine->ObjectTransformsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).Transform = result;
        else
          Engine->ObjectTransformsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).Transform = mth::matr4f::Identity();
      }

      mth::vec3f minAABB = Engine->ObjectTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).minAABB;
      mth::vec3f maxAABB = Engine->ObjectTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).maxAABB;

      if (ImGui::DragFloat3("minAABB", &minAABB.X, 0.1f))
      {
        Engine->ObjectTransformsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).minAABB = minAABB;
      }

      if (ImGui::DragFloat3("maxAABB", &maxAABB.X, 0.1f))
      {
        Engine->ObjectTransformsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).maxAABB = maxAABB;
      }
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceNodeTransform(void)
  {
    ImGui::Text("Node Transform Editor");
    if (Engine->NodeTransformsSystem->IsExist(ParentUnit->IndicesVars["ChoosedElement"]))
    {
      ImGui::Text("Parent index %d", Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ParentIndex);
      if (Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ParentIndex != NONE_INDEX)
      {
        ImGui::SameLine();
        if (ImGui::Button("Go to parent"))
        {
          ParentUnit->IndicesVars["ChoosedElement"] = Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ParentIndex;
          return;
        }
      }

      ImGui::Text("First Child index %d", Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ChildIndex);
      if (Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ChildIndex != NONE_INDEX)
      {
        ImGui::SameLine();
        if (ImGui::Button("Go to first children"))
        {
          ParentUnit->IndicesVars["ChoosedElement"] = Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ChildIndex;
          return;
        }
      }

      ImGui::Text("Next sibling index %d", Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).NextIndex);
      if (Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).NextIndex != NONE_INDEX)
      {
        ImGui::SameLine();
        if (ImGui::Button("Go to next sibling"))
        {
          ParentUnit->IndicesVars["ChoosedElement"] = Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).NextIndex;
          return;
        }
      }

      ImGui::Text("Local Transform");

      mth::vec3f Translate, Rotate, Scale;
      Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).LocalTransform.Decompose(Translate, Rotate, Scale);

      bool IsMatrChanged = false;

      if (ImGui::DragFloat3("Translation", &Translate.X, 0.1f))
        IsMatrChanged = true;
      if (ImGui::DragFloat3("Rotation", &Rotate.X, 0.1f))
        IsMatrChanged = true;
      if (ImGui::DragFloat3("Scale", &Scale.X, 0.1f))
        IsMatrChanged = true;

      if (IsMatrChanged)
      {
        mth::matr4f result = mth::matr4f::RotateZ(Rotate.Z) * mth::matr4f::RotateY(Rotate.Y) * mth::matr4f::RotateX(Rotate.X) * mth::matr4f::Scale(Scale) * mth::matr4f::Translate(Translate);
        if (!isnan(result[0][0]) && !isnan(result[1][1]) && !isnan(result[2][2]) && !isnan(result[3][3]))
          Engine->NodeTransformsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).LocalTransform = result;
        else
          Engine->NodeTransformsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]).LocalTransform = mth::matr4f::Identity();
      }

      ImGui::Text("Global Transform");
      Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).GlobalTransform.Decompose(Translate, Rotate, Scale);

      ImGui::Text("Translation: %f %f %f", Translate.X, Translate.Y, Translate.Z);
      ImGui::Text("Rotation: %f %f %f", Rotate.X, Rotate.Y, Rotate.Z);
      ImGui::Text("Scale: %f %f %f", Scale.X, Scale.Y, Scale.Z);

      ImGui::Text("Bone offset");
      Engine->NodeTransformsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).BoneOffset.Decompose(Translate, Rotate, Scale);

      ImGui::Text("Translation: %f %f %f", Translate.X, Translate.Y, Translate.Z);
      ImGui::Text("Rotation: %f %f %f", Rotate.X, Rotate.Y, Rotate.Z);
      ImGui::Text("Scale: %f %f %f", Scale.X, Scale.Y, Scale.Z);
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceTexture(void)
  {
    ImGui::Text("Texture Editor");
    if (Engine->TexturesSystem->IsExist(ParentUnit->IndicesVars["ChoosedElement"]))
    {
      ImGui::Text("Name: %s", Engine->TexturesSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).Name.c_str());
      ImGui::Text("W: %d", Engine->TexturesSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).W);
      ImGui::Text("H: %d", Engine->TexturesSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).H);
      ImGui::Text("Mips: %d", Engine->TexturesSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).NumOfMips);
      ImGui::Text("Transparent: %s", Engine->TexturesSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).IsTransparent ? "TRUE" : "FALSE");

      D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
      true_texture_handle.ptr += ParentUnit->IndicesVars["ChoosedElement"] * Engine->GetDevice().GetSRVDescSize();

      float aspect = 1.0f * Engine->TexturesSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).H / Engine->TexturesSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).W;

      ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2(128.0f, 128.0f * aspect));
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceCubeTexture(void)
  {
    ImGui::Text("CubeTexture Editor");
    if (Engine->CubeTexturesSystem->IsExist(ParentUnit->IndicesVars["ChoosedElement"]))
    {
      ImGui::Text("Name: %s", Engine->CubeTexturesSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).Name.c_str());
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceShadowMap(void)
  {
    ImGui::Text("Shadow Map Editor");
    if (Engine->ShadowMapsSystem->IsExist(ParentUnit->IndicesVars["ChoosedElement"]))
    {
      ImGui::Text("W: %d", Engine->ShadowMapsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).W);
      ImGui::Text("H: %d", Engine->ShadowMapsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).H);

      D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->ShadowMapsSystem->ShadowMapTableGPU;
      true_texture_handle.ptr += ParentUnit->IndicesVars["ChoosedElement"] * Engine->GetDevice().GetSRVDescSize();

      float aspect = 1.0f * Engine->ShadowMapsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).H / Engine->ShadowMapsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).W;

      ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2(128.0f, 128.0f * aspect));
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceBoneMapping(void)
  {
    ImGui::Text("Bone Mapping Editor");
    if (Engine->BoneMappingSystem->IsExist(ParentUnit->IndicesVars["ChoosedElement"]))
    {
      ImGui::BeginTable("Values", 2, ImGuiTableFlags_Borders);
      ImGui::TableNextColumn();
      ImGui::Text("Geom index");
      ImGui::TableNextColumn();
      ImGui::Text("Node index");
      ImGui::TableNextRow();

      for (int i = 0; i < MAX_BONE_PER_MESH; i++)
      {
        ImGui::TableNextColumn();
        ImGui::Text("%d", i);
        ImGui::TableNextColumn();
        ImGui::Text("%d", Engine->BoneMappingSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).BoneMapping[i]);
        ImGui::TableNextRow();
      }
      ImGui::EndTable();
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceMaterial(void)
  {
    ImGui::Text("Material Editor");
    if (Engine->MaterialsSystem->IsExist(ParentUnit->IndicesVars["ChoosedElement"]))
    {
      const char* items[] = { "Color" , "Phong", "PBR Metal/Rough", "PBR Shiness/Glossiness" };
      ImGui::Combo("Type", (int*)&Engine->MaterialsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ShadeType, items, IM_ARRAYSIZE(items));

      if (Engine->MaterialsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ShadeType == MATERIAL_SHADER_COLOR)
      {
        GDRGPUMaterial& el = Engine->MaterialsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]);
        ImGui::ColorEdit3("Color", &GDRGPUMaterialColorGetColor(el).X);
        ImGui::Text("Opacity %g", GDRGPUMaterialColorGetOpacity(el));
        ImGui::Text("Color texture index %d", GDRGPUMaterialColorGetColorMapIndex(el));
        if (GDRGPUMaterialColorGetColorMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to color texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialColorGetColorMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }
      }

      if (Engine->MaterialsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ShadeType == MATERIAL_SHADER_PHONG)
      {
        GDRGPUMaterial& el = Engine->MaterialsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]);
        ImGui::ColorEdit3("Ambient", &GDRGPUMaterialPhongGetAmbient(el).X);
        ImGui::Text("Ambient texture index %d", GDRGPUMaterialPhongGetAmbientMapIndex(el));
        if (GDRGPUMaterialPhongGetAmbientMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to ambient texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialPhongGetAmbientMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::ColorEdit3("Diffuse", &GDRGPUMaterialPhongGetDiffuse(el).X);
        ImGui::Text("Diffuse texture index %d", GDRGPUMaterialPhongGetDiffuseMapIndex(el));
        if (GDRGPUMaterialPhongGetDiffuseMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to diffuse texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialPhongGetDiffuseMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::ColorEdit3("Specular", &GDRGPUMaterialPhongGetSpecular(el).X);
        ImGui::Text("Specular texture index %d", GDRGPUMaterialPhongGetSpecularMapIndex(el));
        if (GDRGPUMaterialPhongGetSpecularMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to specular texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialPhongGetSpecularMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::DragFloat("Shiness", &GDRGPUMaterialPhongGetShiness(el));

        ImGui::Text("Normal map index %d", GDRGPUMaterialPhongGetNormalMapIndex(el));
        if (GDRGPUMaterialPhongGetNormalMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to normal map"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialPhongGetNormalMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }
      }

      if (Engine->MaterialsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ShadeType == MATERIAL_SHADER_COOKTORRANCE_METALNESS)
      {
        GDRGPUMaterial& el = Engine->MaterialsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]);
        ImGui::Text("Ambient occlusion map index %d", GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to ambient texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::ColorEdit3("Albedo", &GDRGPUMaterialCookTorranceGetAlbedo(el).X);
        ImGui::Text("Albedo map index %d", GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to albedo texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::Text("Normal map index %d", GDRGPUMaterialCookTorranceGetNormalMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetNormalMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to normal map"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialCookTorranceGetNormalMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::Text("Opacity %g", GDRGPUMaterialCookTorranceGetOpacity(el));

        ImGui::DragFloat("Roughness", &GDRGPUMaterialCookTorranceGetRoughness(el));
        ImGui::DragFloat("Metalness", &GDRGPUMaterialCookTorranceGetMetalness(el));

        ImGui::Text("Metal/Rough map index %d", GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to metal/rough texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }
      }

      if (Engine->MaterialsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ShadeType == MATERIAL_SHADER_COOKTORRANCE_SPECULAR)
      {
        GDRGPUMaterial& el = Engine->MaterialsSystem->GetEditable(ParentUnit->IndicesVars["ChoosedElement"]);
        ImGui::Text("Ambient occlusion map index %d", GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to ambient texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::ColorEdit3("Albedo", &GDRGPUMaterialCookTorranceGetAlbedo(el).X);
        ImGui::Text("Albedo map index %d", GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to albedo texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::Text("Normal map index %d", GDRGPUMaterialCookTorranceGetNormalMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetNormalMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to normal map"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialCookTorranceGetNormalMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::Text("Opacity %g", GDRGPUMaterialCookTorranceGetOpacity(el));

        ImGui::ColorEdit3("Specular", &GDRGPUMaterialCookTorranceGetSpecular(el).R);
        ImGui::DragFloat("Glossiness", &GDRGPUMaterialCookTorranceGetGlossiness(el));

        ImGui::Text("Specular/gloss map index %d", GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to specular/gloss texture"))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(el);
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::texture;
            return;
          }
        }
      }
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceModel(void)
  {
    ImGui::Text("Model Editor");
    if (Engine->ModelsManager->IsExist(ParentUnit->IndicesVars["ChoosedElement"]))
    {
      if (ImGui::Button("Delete Model"))
      {
        ParentUnit->FloatVars["Remove"] = true;
      }
      ImGui::Text("Name: %s", Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Name.c_str());
      ImGui::Text("Root Transform index: %d", Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform);
      if (Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform != NONE_INDEX)
      {
        if (ImGui::Button("Go to Transform"))
        {
          ParentUnit->IndicesVars["ChoosedElement"] = Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform;
          ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::object_transform;
          return;
        }
      }
      if (Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform != NONE_INDEX)
      {
        mth::vec3f Translate, Rotate, Scale;
        Engine->ObjectTransformsSystem->Get(Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform).Transform.Decompose(Translate, Rotate, Scale);

        bool IsMatrChanged = false;

        if (ImGui::DragFloat3("Translation", &Translate.X, 0.1f))
          IsMatrChanged = true;
        if (ImGui::DragFloat3("Rotation", &Rotate.X, 0.1f))
          IsMatrChanged = true;
        if (ImGui::DragFloat3("Scale", &Scale.X, 0.1f))
          IsMatrChanged = true;

        if (IsMatrChanged)
        {
          mth::matr4f result = mth::matr4f::RotateZ(Rotate.Z) * mth::matr4f::RotateY(Rotate.Y) * mth::matr4f::RotateX(Rotate.X) * mth::matr4f::Scale(Scale) * mth::matr4f::Translate(Translate);
          if (!isnan(result[0][0]) && !isnan(result[1][1]) && !isnan(result[2][2]) && !isnan(result[3][3]))
            Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform).Transform = result;
          else
            Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform).Transform = mth::matr4f::Identity();
        }
      }

      ImGui::BeginTable("Materials", 2, ImGuiTableFlags_Borders);
      ImGui::TableNextColumn();
      ImGui::Text("Index");
      ImGui::TableNextColumn();
      ImGui::Text("Button to go");
      ImGui::TableNextRow();

      for (int i = 0; i < Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.Materials.size(); i++)
      {
        ImGui::TableNextColumn();
        ImGui::Text("%d", Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.Materials[i].value);
        ImGui::TableNextColumn();
        if (Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.Materials[i].value != NONE_INDEX)
        {
          if (ImGui::Button((std::string("Go to material ") + std::to_string(Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.Materials[i].value)).c_str()))
          {
            ParentUnit->IndicesVars["ChoosedElement"] = Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.Materials[i];
            ParentUnit->IndicesVars["ChoosedElement"].type = gdr_index_types::material;
            ImGui::TableNextRow();
            ImGui::EndTable();
            return;
          }
        }
        ImGui::TableNextRow();
      }
      ImGui::EndTable();
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditorResource()
  {
    ImGui::Text("Resource Editor");
    switch (ParentUnit->IndicesVars["ChoosedElement"].type)
    {
    case gdr_index_types::light:
      ShowEditResourceLight();
      break;
    case gdr_index_types::object_transform:
      ShowEditResourceObjectTransform();
      break;
    case gdr_index_types::node_transform:
      ShowEditResourceNodeTransform();
      break;
    case gdr_index_types::texture:
      ShowEditResourceTexture();
      break;
    case gdr_index_types::cube_texture:
      ShowEditResourceCubeTexture();
      break;
    case gdr_index_types::shadow_map:
      ShowEditResourceShadowMap();
      break;
    case gdr_index_types::bone_mapping:
      ShowEditResourceBoneMapping();
      break;
    case gdr_index_types::material:
      ShowEditResourceMaterial();
      break;
    case gdr_index_types::model:
      ShowEditResourceModel();
      break;
    case gdr_index_types::animation:
    case gdr_index_types::physic_body:
    case gdr_index_types::draw_command:
    case gdr_index_types::geometry:
    case gdr_index_types::none:
    default:
      break;
    }
  }

  void Response(void) override
  {
    // Disable all light objects
    Engine->ObjectTransformsSystem->GetEditable(
      Engine->ModelsManager->Get(IndicesVars["PointMesh"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);
    Engine->ObjectTransformsSystem->GetEditable(
      Engine->ModelsManager->Get(IndicesVars["DirMesh"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);
    Engine->ObjectTransformsSystem->GetEditable(
      Engine->ModelsManager->Get(IndicesVars["ConeMesh"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);

    if (FloatVars["Show"] == 0)
      return;

    // Visualize choosed light object
    if ((editor_type)ParentUnit->FloatVars["EditorType"] == editor_type::resource &&
      ParentUnit->IndicesVars["ChoosedElement"].type == gdr_index_types::light &&
      Engine->LightsSystem->IsExist(ParentUnit->IndicesVars["ChoosedElement"]) &&
      Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ObjectTransformIndex != NONE_INDEX)
    {
      // choose right mesh
      gdr_index ObjectType;
      if (Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
        ObjectType = IndicesVars["DirMesh"];
      if (Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
        ObjectType = IndicesVars["ConeMesh"];
      if (Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).LightSourceType == LIGHT_SOURCE_TYPE_POINT)
        ObjectType = IndicesVars["PointMesh"];

      Engine->ObjectTransformsSystem->GetEditable(
        Engine->ModelsManager->Get(ObjectType).Render.RootTransform).Transform =
        Engine->ObjectTransformsSystem->Get(Engine->LightsSystem->Get(ParentUnit->IndicesVars["ChoosedElement"]).ObjectTransformIndex).Transform;
    }

    Engine->AddLambdaForIMGUI([&]() {
      ImGui::Begin("Editor", reinterpret_cast<bool*>(&FloatVars["Show"]));
      
      editor_type type = (editor_type)ParentUnit->FloatVars["EditorType"];
      switch (type)
      {
      case editor_type::camera:
        ShowEditorCamera();
        break;
      case editor_type::resource:
        ShowEditorResource();
        break;
      case editor_type::unit:
        ShowEditorUnit();
        break;
      case editor_type::none:
      default:
        break;
      }
      ImGui::End();
      });
  }

  std::string GetName(void)
  {
    return "unit_editor_window_edit";
  }

  ~unit_editor_window_edit()
  {
    Engine->ModelsManager->Remove(IndicesVars["PointMesh"]);
    Engine->ModelsManager->Remove(IndicesVars["DirMesh"]);
    Engine->ModelsManager->Remove(IndicesVars["ConeMesh"]);
  }
};