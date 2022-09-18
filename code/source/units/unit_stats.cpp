#include "p_header.h"

#include "unit_stats.h"

void unit_stats::Initialize(void)
{
  TransformToolActive = true;
  CurrentTransformToShow = 0;
  MaterialToolActive = true;
  CurrentMaterialToShow = 0;
}

void unit_stats::Response(void)
{
  Engine->AddLambdaForIMGUI(
    [&]()
    {
      if (!TransformToolActive || Engine->TransformsSystem->CPUData.size() == 0)
        return;
      // Create a window called "My First Tool", with a menu bar.
      ImGui::Begin("Transform Viewer", &TransformToolActive, ImGuiWindowFlags_AlwaysAutoResize);
      if (ImGui::Button("<<<"))
      {
        CurrentTransformToShow -= 100;
        CurrentTransformToShow = max(0, CurrentTransformToShow);
        CurrentTransformToShow = min(CurrentTransformToShow, Engine->TransformsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button("<<"))
      {
        CurrentTransformToShow -= 10;
        CurrentTransformToShow = max(0, CurrentTransformToShow);
        CurrentTransformToShow = min(CurrentTransformToShow, Engine->TransformsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button("<"))
      {
        CurrentTransformToShow -= 1;
        CurrentTransformToShow = max(0, CurrentTransformToShow);
        CurrentTransformToShow = min(CurrentTransformToShow, Engine->TransformsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button(">"))
      {
        CurrentTransformToShow += 1;
        CurrentTransformToShow = max(0, CurrentTransformToShow);
        CurrentTransformToShow = min(CurrentTransformToShow, Engine->TransformsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button(">>"))
      {
        CurrentTransformToShow += 10;
        CurrentTransformToShow = max(0, CurrentTransformToShow);
        CurrentTransformToShow = min(CurrentTransformToShow, Engine->TransformsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button(">>>"))
      {
        CurrentTransformToShow += 100;
        CurrentTransformToShow = max(0, CurrentTransformToShow);
        CurrentTransformToShow = min(CurrentTransformToShow, Engine->TransformsSystem->CPUData.size() - 1);
      }
      ImGui::Text("Transform %d", CurrentTransformToShow);
      ImGui::BeginTable("Transform matrix", 4, ImGuiTableFlags_Borders);

      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          ImGui::TableNextColumn();
          if (Engine->TransformsSystem->CPUData[CurrentTransformToShow].transform[i][j] < 0)
            ImGui::Text("%4.3f", Engine->TransformsSystem->CPUData[CurrentTransformToShow].transform[i][j]);
          else
            ImGui::Text("%4.4f", Engine->TransformsSystem->CPUData[CurrentTransformToShow].transform[i][j]);
        }
        ImGui::TableNextRow();
      }

      ImGui::EndTable();
      ImGui::Text("AABB min (%f, %f, %f)",
        Engine->TransformsSystem->CPUData[CurrentTransformToShow].minAABB.X,
        Engine->TransformsSystem->CPUData[CurrentTransformToShow].minAABB.Y,
        Engine->TransformsSystem->CPUData[CurrentTransformToShow].minAABB.Z);
      ImGui::Text("AABB max (%f, %f, %f)",
        Engine->TransformsSystem->CPUData[CurrentTransformToShow].maxAABB.X,
        Engine->TransformsSystem->CPUData[CurrentTransformToShow].maxAABB.Y,
        Engine->TransformsSystem->CPUData[CurrentTransformToShow].maxAABB.Z);

      ImGui::End();
    });

  Engine->AddLambdaForIMGUI(
    [&]()
    {
      if (!MaterialToolActive || Engine->MaterialsSystem->CPUData.size() == 0)
        return;

      // Create a window called "My First Tool", with a menu bar.
      ImGui::Begin("Material Viewer", &MaterialToolActive, ImGuiWindowFlags_AlwaysAutoResize);
      if (ImGui::Button("<<<"))
      {
        CurrentMaterialToShow -= 100;
        CurrentMaterialToShow = max(0, CurrentMaterialToShow);
        CurrentMaterialToShow = min(CurrentMaterialToShow, Engine->MaterialsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button("<<"))
      {
        CurrentMaterialToShow -= 10;
        CurrentMaterialToShow = max(0, CurrentMaterialToShow);
        CurrentMaterialToShow = min(CurrentMaterialToShow, Engine->MaterialsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button("<"))
      {
        CurrentMaterialToShow -= 1;
        CurrentMaterialToShow = max(0, CurrentMaterialToShow);
        CurrentMaterialToShow = min(CurrentMaterialToShow, Engine->MaterialsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button(">"))
      {
        CurrentMaterialToShow += 1;
        CurrentMaterialToShow = max(0, CurrentMaterialToShow);
        CurrentMaterialToShow = min(CurrentMaterialToShow, Engine->MaterialsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button(">>"))
      {
        CurrentMaterialToShow += 10;
        CurrentMaterialToShow = max(0, CurrentMaterialToShow);
        CurrentMaterialToShow = min(CurrentMaterialToShow, Engine->MaterialsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button(">>>"))
      {
        CurrentMaterialToShow += 100;
        CurrentMaterialToShow = max(0, CurrentMaterialToShow);
        CurrentMaterialToShow = min(CurrentMaterialToShow, Engine->MaterialsSystem->CPUData.size() - 1);
      }
      ImGui::Text("Material %d", CurrentMaterialToShow);
      // Edit a color (stored as ~4 floats)
      ImGui::ColorEdit3("Ka", &Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].Ka[0]);
      ImGui::Text("Ka Texture index %d", Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KaMapIndex);
      if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KaMapIndex != -1)
      {
        D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
        true_texture_handle.ptr += Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KaMapIndex * Engine->GetDevice().GetSRVDescSize();

        ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
      }
      ImGui::ColorEdit3("Kd", &Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].Kd[0]);
      ImGui::Text("Kd Texture index %d", Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KdMapIndex);
      if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KdMapIndex != -1)
      {
        D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
        true_texture_handle.ptr += Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KdMapIndex * Engine->GetDevice().GetSRVDescSize();

        ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
      }
      ImGui::ColorEdit3("Ks", &Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].Ks[0]);
      ImGui::Text("Ks Texture index %d", Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KsMapIndex);
      if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KsMapIndex != -1)
      {
        D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
        true_texture_handle.ptr += Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KsMapIndex * Engine->GetDevice().GetSRVDescSize();

        ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
      }
      ImGui::End();
    });
}