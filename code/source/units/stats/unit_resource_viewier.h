#pragma once
#pragma once
#include "../unit_base.h"

class unit_resource_viewier : public gdr::unit_base
{
private:
  bool MainWindow;
  
  bool DrawCommandsWindow = false;
  gdr_index DrawCommandsIndex = 0;

  bool GlobalsWindow = false;

  bool MaterialsWindow = false;
  gdr_index MaterialsIndex = 0;

  bool TexturesWindow = false;
  gdr_index TexturesIndex = 0;

  bool RenderTargetWindow = false;
  gdr_index RenderTargetIndex = 0;

  bool ObjectTransformsWindow = false;
  gdr_index ObjectTransformIndex = 0;

  bool NodeTransformsWindow = false;
  gdr_index NodeTransformIndex = 0;

  bool ShadowMapsWindow = false;
  gdr_index ShadowMapIndex = 0;

public:
  void Initialize()
  {
  }

  void Response(void)
  {
    if (Engine->KeysClick[VK_OEM_3]) // TILDE
    {
      MainWindow = true;
    }
    Engine->AddLambdaForIMGUI(
      [&]()
      {
        ImGui::Begin("GPU Resource Viewer", &MainWindow, ImGuiWindowFlags_AlwaysAutoResize);
        DrawCommandsWindow |= ImGui::Button("Draw commands");
        GlobalsWindow |= ImGui::Button("Globals");
        MaterialsWindow |= ImGui::Button("Materials");
        TexturesWindow |= ImGui::Button("Textures");
        RenderTargetWindow |= ImGui::Button("Render targets");
        ObjectTransformsWindow |= ImGui::Button("Object transforms");
        NodeTransformsWindow |= ImGui::Button("Node transforms");
        ShadowMapsWindow |= ImGui::Button("Shadow maps");
        ImGui::End();
      }
      );

    if (DrawCommandsWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Draw commands Viewer", &DrawCommandsWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            DrawCommandsIndex.value -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            DrawCommandsIndex.value -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            DrawCommandsIndex.value -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", DrawCommandsIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            DrawCommandsIndex.value += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            DrawCommandsIndex.value += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            DrawCommandsIndex.value += 100;
          DrawCommandsIndex.value = min(max(0, DrawCommandsIndex.value), (unsigned)(Engine->DrawCommandsSystem->AllocatedSize() - 1));

          if (Engine->DrawCommandsSystem->IsExist(DrawCommandsIndex))
          {
          auto &el = Engine->DrawCommandsSystem->Get(DrawCommandsIndex);

          ImGui::Text("Triangles: %d", el.DrawArguments.IndexCountPerInstance / 3);
          if (el.Indices.ObjectMaterialIndex == NONE_INDEX)
            ImGui::Text("Material: NONE");
          else
          {
            ImGui::Text("Material: %d", el.Indices.ObjectMaterialIndex);
            ImGui::SameLine();
            if (ImGui::Button("Go to material"))
            {
              MaterialsWindow = true;
              MaterialsIndex = el.Indices.ObjectMaterialIndex;
            }
          }

          if (el.Indices.ObjectTransformIndex == NONE_INDEX)
            ImGui::Text("Object Transform: NONE");
          else
          {
            ImGui::Text("Object Transform: %d", el.Indices.ObjectTransformIndex);
            ImGui::SameLine();
            if (ImGui::Button("Go to transform"))
            {
              ObjectTransformsWindow = true;
              ObjectTransformIndex = el.Indices.ObjectTransformIndex;
            }
          }

          if (el.Indices.ObjectParamsMask == NONE_INDEX)
            ImGui::Text("Object Params: NONE");
          }
          else
          {
            ImGui::Text("Not exist");
          }
          ImGui::End();
        }
        );

    if (GlobalsWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Globals Viewer", &GlobalsWindow, ImGuiWindowFlags_AlwaysAutoResize);
          
          auto &el = Engine->GlobalsSystem->Get();
          ImGui::Text("Camera position: (%f, %f, %f)", el.CameraPos.X, el.CameraPos.Y, el.CameraPos.Z);
          ImGui::Text("Time: %f", el.Time);
          ImGui::Text("Delta Time: %f", el.DeltaTime);
          ImGui::Text("Screen size: %dx%d", el.Width, el.Height);
          ImGui::End();
        }
        );

    if (MaterialsWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Material Viewer", &MaterialsWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            MaterialsIndex.value -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            MaterialsIndex.value -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            MaterialsIndex.value -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", MaterialsIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            MaterialsIndex.value += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            MaterialsIndex.value += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            MaterialsIndex.value += 100;
          MaterialsIndex.value = min(max(0, MaterialsIndex.value), (unsigned)(Engine->MaterialsSystem->AllocatedSize() - 1));

          if (Engine->MaterialsSystem->IsExist(MaterialsIndex))
          {
            auto& el = Engine->MaterialsSystem->Get(MaterialsIndex);
            if (el.ShadeType == MATERIAL_SHADER_COLOR)
            {
              ImGui::Text("ShadeType: COLOR");
              mth::vec3f color = GDRGPUMaterialColorGetColor(el);
              ImGui::Text("Color: %f %f %f", color.X, color.Y, color.Z);
              if (GDRGPUMaterialColorGetColorMapIndex(el) == NONE_INDEX)
                ImGui::Text("Color Map Index : NONE");
              else
              {
                ImGui::Text("Color Texture Index : %d", GDRGPUMaterialColorGetColorMapIndex(el));
                ImGui::SameLine();
                if (ImGui::Button("Go to color texture"))
                {
                  TexturesWindow = true;
                  TexturesIndex = GDRGPUMaterialColorGetColorMapIndex(el);
                }
              }
            }
            else if (el.ShadeType == MATERIAL_SHADER_PHONG)
            {
                ImGui::Text("ShadeType: PHONG");
                
                mth::vec3f color = GDRGPUMaterialPhongGetAmbient(el);
                ImGui::Text("Ambient: %f %f %f", color.X, color.Y, color.Z);
                if (GDRGPUMaterialPhongGetAmbientMapIndex(el) == NONE_INDEX)
                    ImGui::Text("Ambient Map Index : NONE");
                else
                {
                    ImGui::Text("Ambient Texture Index : %d", GDRGPUMaterialPhongGetAmbientMapIndex(el));
                    ImGui::SameLine();
                    if (ImGui::Button("Go to ambient texture"))
                    {
                        TexturesWindow = true;
                        TexturesIndex = GDRGPUMaterialPhongGetAmbientMapIndex(el);
                    }
                }

                color = GDRGPUMaterialPhongGetDiffuse(el);
                ImGui::Text("Diffuse: %f %f %f", color.X, color.Y, color.Z);
                if (GDRGPUMaterialPhongGetDiffuseMapIndex(el) == NONE_INDEX)
                    ImGui::Text("Diffuse Map Index : NONE");
                else
                {
                    ImGui::Text("Diffuse Texture Index : %d", GDRGPUMaterialPhongGetDiffuseMapIndex(el));
                    ImGui::SameLine();
                    if (ImGui::Button("Go to diffuse texture"))
                    {
                        TexturesWindow = true;
                        TexturesIndex = GDRGPUMaterialPhongGetDiffuseMapIndex(el);
                    }
                }

                color = GDRGPUMaterialPhongGetSpecular(el);
                ImGui::Text("Specular: %f %f %f", color.X, color.Y, color.Z);
                if (GDRGPUMaterialPhongGetSpecularMapIndex(el) == NONE_INDEX)
                    ImGui::Text("Specular Map Index : NONE");
                else
                {
                    ImGui::Text("Specular Texture Index : %d", GDRGPUMaterialPhongGetSpecularMapIndex(el));
                    ImGui::SameLine();
                    if (ImGui::Button("Go to specular texture"))
                    {
                        TexturesWindow = true;
                        TexturesIndex = GDRGPUMaterialPhongGetSpecularMapIndex(el);
                    }
                }

                float Ph = GDRGPUMaterialPhongGetShiness(el);
                ImGui::Text("Shiness: %f", Ph);
                if (GDRGPUMaterialPhongGetNormalMapIndex(el) == NONE_INDEX)
                    ImGui::Text("Normal Map Index : NONE");
                else
                {
                    ImGui::Text("Normal map Texture Index : %d", GDRGPUMaterialPhongGetNormalMapIndex(el));
                    ImGui::SameLine();
                    if (ImGui::Button("Go to Normal map texture"))
                    {
                        TexturesWindow = true;
                        TexturesIndex = GDRGPUMaterialPhongGetNormalMapIndex(el);
                    }
                }
            }
            else if (el.ShadeType == MATERIAL_SHADER_COOKTORRANCE_METALNESS || el.ShadeType == MATERIAL_SHADER_COOKTORRANCE_SPECULAR)
            {
                ImGui::Text(el.ShadeType == MATERIAL_SHADER_COOKTORRANCE_METALNESS ? "ShadeType: CookTorrance Metalness/Roughness" : "ShadeType: CookTorrance Specular/Glossines");

                mth::vec3f color = GDRGPUMaterialCookTorranceGetAlbedo(el);
                ImGui::Text("Albedo: %f %f %f", color.X, color.Y, color.Z);
                if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el) == NONE_INDEX)
                    ImGui::Text("Albedo Map Index : NONE");
                else
                {
                    ImGui::Text("Albedo Texture Index : %d", GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el));
                    ImGui::SameLine();
                    if (ImGui::Button("Go to albedo texture"))
                    {
                        TexturesWindow = true;
                        TexturesIndex = GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el);
                    }
                }
                if (el.ShadeType == MATERIAL_SHADER_COOKTORRANCE_METALNESS)
                {
                    float metalness = GDRGPUMaterialCookTorranceGetMetalness(el);
                    ImGui::Text("Metalness: %f", metalness);
                    float roughness = GDRGPUMaterialCookTorranceGetRoughness(el);
                    ImGui::Text("Roughness: %f", roughness);
                    if (GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(el) == NONE_INDEX)
                        ImGui::Text("Metalness Map Index : NONE");
                    else
                    {
                        ImGui::Text("Metalness Texture Index : %d", GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(el));
                        ImGui::SameLine();
                        if (ImGui::Button("Go to metalness texture"))
                        {
                            TexturesWindow = true;
                            TexturesIndex = GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(el);
                        }
                    }
                }
                else
                {
                    float3 specular = GDRGPUMaterialCookTorranceGetSpecular(el);
                    ImGui::Text("Specular: %f %f %f", specular.X, specular.Y, specular.Z);
                    float glossiness = GDRGPUMaterialCookTorranceGetGlossiness(el);
                    ImGui::Text("Glossines: %f", glossiness);
                    if (GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(el) == NONE_INDEX)
                        ImGui::Text("Specular/Glossiness Map Index : NONE");
                    else
                    {
                        ImGui::Text("Specular/Glossiness Texture Index : %d", GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(el));
                        ImGui::SameLine();
                        if (ImGui::Button("Go to Specular/Glossiness texture"))
                        {
                            TexturesWindow = true;
                            TexturesIndex = GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(el);
                        }
                    }
                }
                if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el) == NONE_INDEX)
                    ImGui::Text("Ambient occlusion Map Index : NONE");
                else
                {
                    ImGui::Text("Ambient occlusion Texture Index : %d", GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el));
                    ImGui::SameLine();
                    if (ImGui::Button("Go to Ambient occlusion texture"))
                    {
                        TexturesWindow = true;
                        TexturesIndex = GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el);
                    }
                }

                if (GDRGPUMaterialCookTorranceGetNormalMapIndex(el) == NONE_INDEX)
                    ImGui::Text("Normal Map Index : NONE");
                else
                {
                    ImGui::Text("Normal map Texture Index : %d", GDRGPUMaterialCookTorranceGetNormalMapIndex(el));
                    ImGui::SameLine();
                    if (ImGui::Button("Go to Normal map texture"))
                    {
                        TexturesWindow = true;
                        TexturesIndex = GDRGPUMaterialCookTorranceGetNormalMapIndex(el);
                    }
                }
            }
            else
              ImGui::Text("ShadeType: Unknown");
          }
          else
          {
            ImGui::Text("Not exist");
          }
          ImGui::End();
        }
        );

    if (TexturesWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Texture Viewer", &TexturesWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            TexturesIndex.value -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            TexturesIndex.value -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            TexturesIndex.value -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", TexturesIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            TexturesIndex.value += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            TexturesIndex.value += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            TexturesIndex.value += 100;
          TexturesIndex.value = min(max(0, TexturesIndex.value), (unsigned)(Engine->TexturesSystem->AllocatedSize() - 1));

          if (Engine->TexturesSystem->IsExist(TexturesIndex))
          {
            auto& el = Engine->TexturesSystem->Get(TexturesIndex);
            ImGui::Text("Name  : %s", el.Name.c_str());
            ImGui::Text("Width : %d", el.W);
            ImGui::Text("Heigth: %d", el.H);
            ImGui::Text("Mips  : %d", el.NumOfMips);
            ImGui::Text("Transparency : %s", el.IsTransparent ? "TRUE" : "FALSE");

            D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
            true_texture_handle.ptr += TexturesIndex * Engine->GetDevice().GetSRVDescSize();
            ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));

          }
          else
          {
            ImGui::Text("Not exist");
          }

          ImGui::End();
        }
        );

    if (ShadowMapsWindow)
        Engine->AddLambdaForIMGUI(
            [&]()
            {
                ImGui::Begin("Shadow maps Viewer", &ShadowMapsWindow, ImGuiWindowFlags_AlwaysAutoResize);
                if (ImGui::Button("<<<"))
                    ShadowMapIndex.value -= 100;
                ImGui::SameLine();
                if (ImGui::Button("<<"))
                    ShadowMapIndex.value -= 10;
                ImGui::SameLine();
                if (ImGui::Button("<"))
                    ShadowMapIndex.value -= 1;
                ImGui::SameLine();
                ImGui::Text("%d", ShadowMapIndex);
                ImGui::SameLine();
                if (ImGui::Button(">"))
                    ShadowMapIndex.value += 1;
                ImGui::SameLine();
                if (ImGui::Button(">>"))
                    ShadowMapIndex.value += 10;
                ImGui::SameLine();
                if (ImGui::Button(">>>"))
                    ShadowMapIndex.value += 100;
                ShadowMapIndex.value = min(max(0, ShadowMapIndex.value), (unsigned)(Engine->ShadowMapsSystem->AllocatedSize() - 1));

                if (Engine->ShadowMapsSystem->IsExist(ShadowMapIndex))
                {
                    auto& el = Engine->ShadowMapsSystem->Get(ShadowMapIndex);
                    ImGui::Text("Width : %d", el.W);
                    ImGui::Text("Heigth: %d", el.H);
                    
                    D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->ShadowMapsSystem->ShadowMapTableGPU;
                    true_texture_handle.ptr += ShadowMapIndex * Engine->GetDevice().GetSRVDescSize();
                    ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
                }
                else
                {
                    ImGui::Text("Not exist");
                }

                ImGui::End();
            }
    );

    if (RenderTargetWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Render Targets Viewer", &RenderTargetWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            RenderTargetIndex.value -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            RenderTargetIndex.value -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            RenderTargetIndex.value -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", RenderTargetIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            RenderTargetIndex.value += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            RenderTargetIndex.value += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            RenderTargetIndex.value += 100;
          RenderTargetIndex.value = min(max(0, RenderTargetIndex.value), (int)gdr::render_targets_enum::target_count - 1);

          auto& el = Engine->RenderTargetsSystem->Textures[RenderTargetIndex];

          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->RenderTargetsSystem->ShaderResourceViewsGPU;
          true_texture_handle.ptr += RenderTargetIndex * Engine->GetDevice().GetSRVDescSize();
          // width over height
          float ratio = 1.0f * Engine->GlobalsSystem->Get().Width / Engine->GlobalsSystem->Get().Height;
          ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2(ratio * 128.0f, 128.0f));

          ImGui::End();
        }
        );

    if (ObjectTransformsWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Object Transform Viewer", &ObjectTransformsWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            ObjectTransformIndex.value -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            ObjectTransformIndex.value -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            ObjectTransformIndex.value -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", ObjectTransformIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            ObjectTransformIndex.value += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            ObjectTransformIndex.value += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            ObjectTransformIndex.value += 100;
          ObjectTransformIndex.value = min(max(0, ObjectTransformIndex.value), (unsigned)Engine->ObjectTransformsSystem->AllocatedSize() - 1);

          if (Engine->ObjectTransformsSystem->IsExist(ObjectTransformIndex))
          {
            auto& el = Engine->ObjectTransformsSystem->Get(ObjectTransformIndex);

            ImGui::Text("Matrix:");
            for (int i = 0; i < 4; i++)
            {
              ImGui::Text("\t");
              ImGui::SameLine();
              for (int j = 0; j < 4; j++)
              {
                ImGui::Text("%f ", el.Transform[i][j]);
                ImGui::SameLine();
              }
              ImGui::Text("");
            }

            ImGui::Text("OBB:");
            ImGui::Text("\tmin %f %f %f", el.minAABB.X, el.minAABB.Y, el.minAABB.Z);
            ImGui::Text("\tmax %f %f %f", el.maxAABB.X, el.maxAABB.Y, el.maxAABB.Z);
            ImGui::End();
          }
          else
          {
            ImGui::Text("Not exist");
          }
        }
        );

    if (NodeTransformsWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Node Transform Viewer", &NodeTransformsWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            NodeTransformIndex.value -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            NodeTransformIndex.value -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            NodeTransformIndex.value -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", NodeTransformIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            NodeTransformIndex.value += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            NodeTransformIndex.value += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            NodeTransformIndex.value += 100;
          NodeTransformIndex.value = min(max(0, NodeTransformIndex.value), (unsigned)Engine->NodeTransformsSystem->AllocatedSize() - 1);

          if (Engine->NodeTransformsSystem->IsExist(NodeTransformIndex))
          {
          auto& el = Engine->NodeTransformsSystem->Get(NodeTransformIndex);

          ImGui::Text("Local Matrix:");
          for (int i = 0; i < 4; i++)
          {
            ImGui::Text("\t");
            ImGui::SameLine();
            for (int j = 0; j < 4; j++)
            {
              ImGui::Text("%f ", el.LocalTransform[i][j]);
              ImGui::SameLine();
            }
            ImGui::Text("");
          }

          ImGui::Text("Bone Matrix:");
          for (int i = 0; i < 4; i++)
          {
            ImGui::Text("\t");
            ImGui::SameLine();
            for (int j = 0; j < 4; j++)
            {
              ImGui::Text("%f ", el.BoneOffset[i][j]);
              ImGui::SameLine();
            }
            ImGui::Text("");
          }

          ImGui::Text("Global Matrix:");
          for (int i = 0; i < 4; i++)
          {
            ImGui::Text("\t");
            ImGui::SameLine();
            for (int j = 0; j < 4; j++)
            {
              ImGui::Text("%f ", el.GlobalTransform[i][j]);
              ImGui::SameLine();
            }
            ImGui::Text("");
          }

          if (el.ParentIndex == NONE_INDEX)
            ImGui::Text("Parent : NONE");
          else
          {
            ImGui::Text("Parent: %d", el.ParentIndex);
            ImGui::SameLine();
            if (ImGui::Button("Go to parent transform"))
              NodeTransformIndex = el.ParentIndex;
          }

          if (el.NextIndex == NONE_INDEX)
            ImGui::Text("Next sibling : NONE");
          else
          {
            ImGui::Text("Next sibling: %d", el.NextIndex);
            ImGui::SameLine();
            if (ImGui::Button("Go to next sibling transform"))
              NodeTransformIndex = el.NextIndex;
          }

          if (el.ChildIndex == NONE_INDEX)
            ImGui::Text("First Child: NONE");
          else
          {
            ImGui::Text("First Child: %d", el.ChildIndex);
            ImGui::SameLine();
            if (ImGui::Button("Go to first child transform"))
              NodeTransformIndex = el.ChildIndex;
          }
          }
          else
          {
            ImGui::Text("Not exist");
          }
          ImGui::End();
        }
    );
  }

  std::string GetName(void)
  {
    return "unit_resource_viewier";
  }

  ~unit_resource_viewier(void)
  {
  }
};