#pragma once
#pragma once
#include "../unit_base.h"

class unit_resource_viewier : public gdr::unit_base
{
private:
  bool MainWindow;
  
  bool DrawCommandsWindow = false;
  int DrawCommandsIndex = 0;

  bool GlobalsWindow = false;

  bool MaterialsWindow = false;
  int MaterialsIndex = 0;

  bool TexturesWindow = false;
  int TexturesIndex = 0;

  bool RenderTargetWindow = false;
  int RenderTargetIndex = 0;

  bool ObjectTransformsWindow = false;
  int ObjectTransformIndex = 0;

  bool NodeTransformsWindow = false;
  int NodeTransformIndex = 0;

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
        ImGui::End();
      }
      );

    if (DrawCommandsWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Draw commands Viewer", &DrawCommandsWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            DrawCommandsIndex -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            DrawCommandsIndex -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            DrawCommandsIndex -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", DrawCommandsIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            DrawCommandsIndex += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            DrawCommandsIndex += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            DrawCommandsIndex += 100;
          DrawCommandsIndex = min(max(0, DrawCommandsIndex), Engine->DrawCommandsSystem->CPUData.size() - 1);

          auto &el = Engine->DrawCommandsSystem->CPUData[DrawCommandsIndex];

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
          ImGui::End();
        }
        );

    if (GlobalsWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Globals Viewer", &GlobalsWindow, ImGuiWindowFlags_AlwaysAutoResize);
          
          auto &el = Engine->GlobalsSystem->CPUData;
          ImGui::Text("Camera position: (%f, %f, %f)", el.CameraPos.X, el.CameraPos.Y, el.CameraPos.Z);
          ImGui::Text("Time: %f", el.time);
          ImGui::Text("Delta Time: %f", el.DeltaTime);
          ImGui::Text("Screen size: %dx%d", el.width, el.height);
          ImGui::End();
        }
        );

    if (MaterialsWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Material Viewer", &MaterialsWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            MaterialsIndex -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            MaterialsIndex -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            MaterialsIndex -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", MaterialsIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            MaterialsIndex += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            MaterialsIndex += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            MaterialsIndex += 100;
          MaterialsIndex = min(max(0, MaterialsIndex), Engine->MaterialsSystem->CPUData.size() - 1);

          auto& el = Engine->MaterialsSystem->CPUData[MaterialsIndex];
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
          else
            ImGui::Text("ShadeType: Unknown");
          ImGui::End();
        }
        );

    if (TexturesWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Texture Viewer", &TexturesWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            TexturesIndex -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            TexturesIndex -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            TexturesIndex -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", TexturesIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            TexturesIndex += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            TexturesIndex += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            TexturesIndex += 100;
          TexturesIndex = min(max(0, TexturesIndex), Engine->TexturesSystem->CPUData.size() - 1);

          auto& el = Engine->TexturesSystem->CPUData[TexturesIndex];
          ImGui::Text("Name  : %s", el.Name.c_str());
          ImGui::Text("Width : %d", el.W);
          ImGui::Text("Heigth: %d", el.H);
          ImGui::Text("Mips  : %d", el.NumOfMips);
          ImGui::Text("Transparency : %s", el.IsTransparent ? "TRUE" : "FALSE");

          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
          true_texture_handle.ptr += TexturesIndex * Engine->GetDevice().GetSRVDescSize();
          ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));

          ImGui::End();
        }
        );

    if (RenderTargetWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Render Targets Viewer", &RenderTargetWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            RenderTargetIndex -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            RenderTargetIndex -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            RenderTargetIndex -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", RenderTargetIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            RenderTargetIndex += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            RenderTargetIndex += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            RenderTargetIndex += 100;
          RenderTargetIndex = min(max(0, RenderTargetIndex), (int)gdr::render_targets_enum::target_count - 1);

          auto& el = Engine->RenderTargetsSystem->Textures[RenderTargetIndex];

          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->RenderTargetsSystem->ShaderResourceViewsGPU;
          true_texture_handle.ptr += RenderTargetIndex * Engine->GetDevice().GetSRVDescSize();
          // width over height
          float ratio = Engine->GlobalsSystem->CPUData.width / Engine->GlobalsSystem->CPUData.height;
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
            ObjectTransformIndex -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            ObjectTransformIndex -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            ObjectTransformIndex -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", ObjectTransformIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            ObjectTransformIndex += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            ObjectTransformIndex += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            ObjectTransformIndex += 100;
          ObjectTransformIndex = min(max(0, ObjectTransformIndex), (int)Engine->ObjectTransformsSystem->CPUData.size() - 1);

          auto& el = Engine->ObjectTransformsSystem->CPUData[ObjectTransformIndex];

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
        );

    if (NodeTransformsWindow)
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Node Transform Viewer", &NodeTransformsWindow, ImGuiWindowFlags_AlwaysAutoResize);
          if (ImGui::Button("<<<"))
            NodeTransformIndex -= 100;
          ImGui::SameLine();
          if (ImGui::Button("<<"))
            NodeTransformIndex -= 10;
          ImGui::SameLine();
          if (ImGui::Button("<"))
            NodeTransformIndex -= 1;
          ImGui::SameLine();
          ImGui::Text("%d", NodeTransformIndex);
          ImGui::SameLine();
          if (ImGui::Button(">"))
            NodeTransformIndex += 1;
          ImGui::SameLine();
          if (ImGui::Button(">>"))
            NodeTransformIndex += 10;
          ImGui::SameLine();
          if (ImGui::Button(">>>"))
            NodeTransformIndex += 100;
          NodeTransformIndex = min(max(0, NodeTransformIndex), (int)Engine->NodeTransformsSystem->CPUData.size() - 1);

          auto& el = Engine->NodeTransformsSystem->CPUData[NodeTransformIndex];

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