#include "p_header.h"

#include "unit_stats.h"

void unit_stats::Initialize(void)
{
  TransformToolActive = false;
  CurrentTransformToShow = 0;
  MaterialToolActive = false;
  CurrentMaterialToShow = 0;
  
  StatsToolActive = true;
  CPURenderTimePlot.resize(PlotWindowWidth);
  DeviceRenderTimePlot.resize(PlotWindowWidth);
  GPUMemoryUsagePlot.resize(PlotWindowWidth);
  CurrentPlotIndex = 0;
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

      mth::vec3f Translate, Rotate, Scale;
      Engine->TransformsSystem->CPUData[CurrentTransformToShow].transform.Decompose(Translate, Rotate, Scale);

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
          Engine->TransformsSystem->CPUData[CurrentTransformToShow].transform = result;
        else
          Engine->TransformsSystem->CPUData[CurrentTransformToShow].transform = mth::matr4f::Identity();
      }

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

      const char* items[] = { "Diffuse component only" , "Phong" };
      ImGui::Combo("Used Shader", (int *)&Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].ShadeType, items, IM_ARRAYSIZE(items));

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


  CPURenderTimePlot[CurrentPlotIndex] = Engine->GetGlobalDeltaTime() * 1000.0;
  DeviceRenderTimePlot[CurrentPlotIndex] = Engine->DeviceFrameCounter.GetMSec();

  DXGI_QUERY_VIDEO_MEMORY_INFO gpu_info;
  ((IDXGIAdapter3 *)Engine->GetDevice().GetAdapter())->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &gpu_info);
  GPUMemoryUsagePlot[CurrentPlotIndex] = gpu_info.CurrentUsage / 1024.0 / 1024.0;
  MaxGPUMemoryPlot = gpu_info.Budget / 1024.0 / 1024.0;
  
  AverageCPURenderTimePlot = 0;
  for (int i = 0; i < CPURenderTimePlot.size(); i++)
    AverageCPURenderTimePlot += CPURenderTimePlot[i];
  AverageCPURenderTimePlot /= CPURenderTimePlot.size();

  AverageDeviceRenderTimePlot = 0;
  for (int i = 0; i < DeviceRenderTimePlot.size(); i++)
    AverageDeviceRenderTimePlot += DeviceRenderTimePlot[i];
  AverageDeviceRenderTimePlot /= DeviceRenderTimePlot.size();

  AverageGPUMemoryUsagePlot = 0;
  for (int i = 0; i < GPUMemoryUsagePlot.size(); i++)
    AverageGPUMemoryUsagePlot += GPUMemoryUsagePlot[i];
  AverageGPUMemoryUsagePlot /= GPUMemoryUsagePlot.size();

  CurrentPlotIndex = (CurrentPlotIndex + 1) % PlotWindowWidth;


  Engine->AddLambdaForIMGUI(
    [&]()
    {
      if (!StatsToolActive)
        return;

      ImGui::Begin("Stats", &StatsToolActive, ImGuiWindowFlags_AlwaysAutoResize);

      if (ImGui::Button("Open Materials viewer"))
      {
        MaterialToolActive = true;
        StatsToolActive = false;
      }

      if (ImGui::Button("Open Transforms viewer"))
      {
        TransformToolActive = true;
        StatsToolActive = false;
      }

      ImGui::Text("Average FPS = %.4g", Engine->GetFPS());
      ImGui::Text("Average CPU Render Time = %.4g ms", AverageCPURenderTimePlot);
      ImGui::Text("Average Device Render Time = %f ms", AverageDeviceRenderTimePlot);
      ImGui::Text("Average GPU Usage = %f MB", AverageGPUMemoryUsagePlot);
      ImGui::Text("Max GPU budget = %f MB", MaxGPUMemoryPlot);
      ImGui::Text("Objects amount = %zd", Engine->ObjectSystem->CPUPool.size());
      ImGui::Text("Geometry amount = %zd", Engine->GeometrySystem->CPUPool.size());
      ImGui::Text("Materials amount = %zd", Engine->MaterialsSystem->CPUData.size());
      ImGui::Text("Max Textures amount = %zd", gdr::MAX_TEXTURE_AMOUNT);
      ImGui::Text("Current project time %g", Engine->GetTime());
      ImGui::Checkbox("Indirect Render", &Engine->Params.IsIndirect);
      if (Engine->Params.IsIndirect)
        ImGui::Checkbox("Culling", &Engine->Params.IsCulling);

      ImGui::Checkbox("Transparents Render", &Engine->Params.IsTransparent);
      bool pause = Engine->GetPause();
      if (ImGui::Checkbox("Pause", &pause))
      {
        Engine->SetPause(pause);
      }

      ImGui::End();
    });


  if (Engine->KeysClick[VK_OEM_3]) // TILDE
  {
    StatsToolActive = true;
  }
}