#include "p_header.h"

#include "unit_stats.h"

void unit_stats::Initialize(void)
{
  TransformToolActive = false;
  CurrentTransformToShow = 0;
  MaterialToolActive = false;
  CurrentMaterialToShow = 0;
  LightToolActive = false;
  CurrentLightToShow = 0;
  
  StatsToolActive = true;
  CPURenderTimePlot.resize(PlotWindowWidth);
  CPUUpdateTimePlot.resize(PlotWindowWidth);
  CPUFrameRenderTimePlot.resize(PlotWindowWidth);
  DeviceRenderTimePlot.resize(PlotWindowWidth);
  GPUMemoryUsagePlot.resize(PlotWindowWidth);
  CurrentPlotIndex = 0;

  PresentMode = false;

  // load all objects for lights
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "unit_stats lights Init");
  PointLightObject = Engine->ObjectSystem->CreateObjectFromFile("bin/models/light_meshes/sphere.obj");
  DirLightObject = Engine->ObjectSystem->CreateObjectFromFile("bin/models/light_meshes/dir.obj");
  SpotLightObject = Engine->ObjectSystem->CreateObjectFromFile("bin/models/light_meshes/cone.obj");
  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_stats::Response(void)
{
  // set all light objects transforms to zero
  Engine->ObjectSystem->NodesPool[PointLightObject].GetTransformEditable() = mth::matr4f::Scale(0);
  Engine->ObjectSystem->NodesPool[SpotLightObject].GetTransformEditable() = mth::matr4f::Scale(0);
  Engine->ObjectSystem->NodesPool[DirLightObject].GetTransformEditable() = mth::matr4f::Scale(0);

  // if light tool is active
  if (LightToolActive)
  {
    // choose right mesh
    gdr::gdr_index ObjectType;
    if (Engine->LightsSystem->GetLight(CurrentLightToShow).LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
      ObjectType = DirLightObject;
    if (Engine->LightsSystem->GetLight(CurrentLightToShow).LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
      ObjectType = SpotLightObject;
    if (Engine->LightsSystem->GetLight(CurrentLightToShow).LightSourceType == LIGHT_SOURCE_TYPE_POINT)
      ObjectType = PointLightObject;

    Engine->ObjectSystem->NodesPool[ObjectType].GetTransformEditable() = Engine->LightsSystem->GetTransform(CurrentLightToShow).transform;
    if (ObjectType == DirLightObject)
      Engine->ObjectSystem->NodesPool[ObjectType].GetTransformEditable() = Engine->ObjectSystem->NodesPool[ObjectType].GetTransformEditable() * mth::matr::Translate({0, 10, 0});
  }
  // Present mode
  Engine->AddLambdaForIMGUI(
    [&]()
    {
          ImGuiIO& io = ImGui::GetIO();
          io.FontGlobalScale = PresentMode ? 1.5 : 1.0;
    });

  // Transform viewer
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

      if (!PresentMode)
      {
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

      }

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

  // Material viewer
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

      const char* items[] = { "Diffuse component only" , "Phong", "Cook Torrance"};
      ImGui::Combo("Used Shader", (int *)&Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].ShadeType, items, IM_ARRAYSIZE(items));

      // Edit a color (stored as ~4 floats)
      if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].ShadeType == MATERIAL_SHADER_DIFFUSE)
      {
        ImGui::ColorEdit3("Diffuse", &Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].Kd[0]);
        ImGui::Text("Diffuse Texture index %d", Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KdMapIndex);
        if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KdMapIndex != -1)
        {
          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
          true_texture_handle.ptr += Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KdMapIndex * Engine->GetDevice().GetSRVDescSize();

          ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
        }
      }

      if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].ShadeType == MATERIAL_SHADER_PHONG)
      {
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
      }
      if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].ShadeType == MATERIAL_SHADER_COOKTORRANCE)
      {
        ImGui::DragFloat("Roughness", &Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].Ks.G, 0.001);
        ImGui::Text("Roughness Texture index %d", Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KsMapIndex);
        if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KsMapIndex != -1)
        {
          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
          true_texture_handle.ptr += Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KsMapIndex * Engine->GetDevice().GetSRVDescSize();

          ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
        }
        ImGui::ColorEdit3("Albedo", &Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].Kd.X);
        ImGui::Text("Albedo Texture index %d", Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KdMapIndex);
        if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KdMapIndex != -1)
        {
          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
          true_texture_handle.ptr += Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KdMapIndex * Engine->GetDevice().GetSRVDescSize();

          ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
        }
        ImGui::DragFloat("Metallic", &Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].Ks.B, 0.001);
        ImGui::Text("Metallic Texture index %d", Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KsMapIndex);
        if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KsMapIndex != -1)
        {
          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
          true_texture_handle.ptr += Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].KsMapIndex * Engine->GetDevice().GetSRVDescSize();

          ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
        }
      }

      ImGui::Text("Normal Texture index %d", Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].NormalMapIndex);
      if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].NormalMapIndex != -1)
      {
        D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
        true_texture_handle.ptr += Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].NormalMapIndex * Engine->GetDevice().GetSRVDescSize();

        ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
      }
      ImGui::Text("Opacity Texture index %d", Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].OpacityMapIndex);
      if (Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].OpacityMapIndex != -1)
      {
          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
          true_texture_handle.ptr += Engine->MaterialsSystem->CPUData[CurrentMaterialToShow].OpacityMapIndex * Engine->GetDevice().GetSRVDescSize();

          ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
      }
      ImGui::End();
    });

  // Lights viewer
  Engine->AddLambdaForIMGUI(
    [&]()
    {
      if (!LightToolActive || Engine->LightsSystem->CPUData.size() == 0)
        return;
      ImGui::Begin("Lights Viewer", &LightToolActive, ImGuiWindowFlags_AlwaysAutoResize);
      if (ImGui::Button("<<<"))
      {
        CurrentLightToShow -= 100;
        CurrentLightToShow = max(0, CurrentLightToShow);
        CurrentLightToShow = min(CurrentLightToShow, Engine->LightsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button("<<"))
      {
        CurrentLightToShow -= 10;
        CurrentLightToShow = max(0, CurrentLightToShow);
        CurrentLightToShow = min(CurrentLightToShow, Engine->LightsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button("<"))
      {
        CurrentLightToShow -= 1;
        CurrentLightToShow = max(0, CurrentLightToShow);
        CurrentLightToShow = min(CurrentLightToShow, Engine->LightsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button(">"))
      {
        CurrentLightToShow += 1;
        CurrentLightToShow = max(0, CurrentLightToShow);
        CurrentLightToShow = min(CurrentLightToShow, Engine->LightsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button(">>"))
      {
        CurrentLightToShow += 10;
        CurrentLightToShow = max(0, CurrentLightToShow);
        CurrentLightToShow = min(CurrentLightToShow, Engine->LightsSystem->CPUData.size() - 1);
      }
      ImGui::SameLine();
      if (ImGui::Button(">>>"))
      {
        CurrentLightToShow += 100;
        CurrentLightToShow = max(0, CurrentLightToShow);
        CurrentLightToShow = min(CurrentLightToShow, Engine->LightsSystem->CPUData.size() - 1);
      }

      if (ImGui::Button("New Light"))
      {
        CurrentLightToShow = Engine->LightsSystem->AddDirectionalLightSource();
      }
      ImGui::SameLine();

      ImGui::Text("Light %d", CurrentLightToShow);

      const char* items[] = { "Directional" , "Point", "Spot"};
      ImGui::Combo("Type", (int*)&Engine->LightsSystem->GetLight(CurrentLightToShow).LightSourceType, items, IM_ARRAYSIZE(items));

      ImGui::Text("Transform index %d", Engine->LightsSystem->GetLight(CurrentLightToShow).ObjectTransformIndex);
      ImGui::SameLine();
      if (ImGui::Button("Open"))
      {
        TransformToolActive = true;
        CurrentTransformToShow = Engine->LightsSystem->GetLight(CurrentLightToShow).ObjectTransformIndex;
      }

      ImGui::ColorEdit3("Color", &Engine->LightsSystem->GetLight(CurrentLightToShow).Color[0]);
      ImGui::DragFloat("Red", &Engine->LightsSystem->GetLight(CurrentLightToShow).Color[0], 0.1);
      ImGui::DragFloat("Blue", &Engine->LightsSystem->GetLight(CurrentLightToShow).Color[1], 0.1);
      ImGui::DragFloat("Green", &Engine->LightsSystem->GetLight(CurrentLightToShow).Color[2], 0.1);

      if (Engine->LightsSystem->GetLight(CurrentLightToShow).LightSourceType != LIGHT_SOURCE_TYPE_DIRECTIONAL)
      {
        ImGui::Text("Attenuation");
        ImGui::DragFloat("Constant part", &Engine->LightsSystem->GetLight(CurrentLightToShow).ConstantAttenuation, 0.1);
        ImGui::DragFloat("Linear part", &Engine->LightsSystem->GetLight(CurrentLightToShow).LinearAttenuation, 0.1);
        ImGui::DragFloat("Quadric part", &Engine->LightsSystem->GetLight(CurrentLightToShow).QuadricAttenuation, 0.1);
        Engine->LightsSystem->GetLight(CurrentLightToShow).ConstantAttenuation = max(1, Engine->LightsSystem->GetLight(CurrentLightToShow).ConstantAttenuation);
      }

      if (Engine->LightsSystem->GetLight(CurrentLightToShow).LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
      {
        Engine->LightsSystem->GetLight(CurrentLightToShow).AngleInnerCone *= MTH_R2D;
        Engine->LightsSystem->GetLight(CurrentLightToShow).AngleOuterCone *= MTH_R2D;
        ImGui::DragFloat("Inner cone angle", &Engine->LightsSystem->GetLight(CurrentLightToShow).AngleInnerCone, 0.1, 1);
        ImGui::DragFloat("Outer cone angle", &Engine->LightsSystem->GetLight(CurrentLightToShow).AngleOuterCone, 0.1, Engine->LightsSystem->GetLight(CurrentLightToShow).AngleInnerCone);
        Engine->LightsSystem->GetLight(CurrentLightToShow).AngleInnerCone = max(5, Engine->LightsSystem->GetLight(CurrentLightToShow).AngleInnerCone);
        Engine->LightsSystem->GetLight(CurrentLightToShow).AngleOuterCone = max(Engine->LightsSystem->GetLight(CurrentLightToShow).AngleInnerCone, Engine->LightsSystem->GetLight(CurrentLightToShow).AngleOuterCone);
        Engine->LightsSystem->GetLight(CurrentLightToShow).AngleInnerCone *= MTH_D2R;
        Engine->LightsSystem->GetLight(CurrentLightToShow).AngleOuterCone *= MTH_D2R;
      }
      ImGui::End();
    });

  CPURenderTimePlot[CurrentPlotIndex] = Engine->GetGlobalDeltaTime() * 1000.0;
  DeviceRenderTimePlot[CurrentPlotIndex] = Engine->DeviceFrameCounter.GetMSec();
  CPUUpdateTimePlot[CurrentPlotIndex] = Engine->UpdateBuffersTime / 1000.0 / 1000.0;
  CPUFrameRenderTimePlot[CurrentPlotIndex] = Engine->DrawFrameTime / 1000.0 / 1000.0;

  DXGI_QUERY_VIDEO_MEMORY_INFO gpu_info;
  ((IDXGIAdapter3 *)Engine->GetDevice().GetAdapter())->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &gpu_info);
  GPUMemoryUsagePlot[CurrentPlotIndex] = gpu_info.CurrentUsage / 1024.0 / 1024.0;
  MaxGPUMemoryPlot = gpu_info.Budget / 1024.0 / 1024.0;
  
  AverageCPURenderTimePlot = 0;
  for (int i = 0; i < CPURenderTimePlot.size(); i++)
    AverageCPURenderTimePlot += CPURenderTimePlot[i];
  AverageCPURenderTimePlot /= CPURenderTimePlot.size();

  AverageCPUUpdateTimePlot = 0;
  for (int i = 0; i < CPUUpdateTimePlot.size(); i++)
      AverageCPUUpdateTimePlot += CPUUpdateTimePlot[i];
  AverageCPUUpdateTimePlot /= CPUUpdateTimePlot.size();

  AverageCPUFrameRenderTimePlot = 0;
  for (int i = 0; i < CPUFrameRenderTimePlot.size(); i++)
      AverageCPUFrameRenderTimePlot += CPUFrameRenderTimePlot[i];
  AverageCPUFrameRenderTimePlot /= CPUFrameRenderTimePlot.size();

  AverageDeviceRenderTimePlot = 0;
  for (int i = 0; i < DeviceRenderTimePlot.size(); i++)
    AverageDeviceRenderTimePlot += DeviceRenderTimePlot[i];
  AverageDeviceRenderTimePlot /= DeviceRenderTimePlot.size();

  AverageGPUMemoryUsagePlot = 0;
  for (int i = 0; i < GPUMemoryUsagePlot.size(); i++)
    AverageGPUMemoryUsagePlot += GPUMemoryUsagePlot[i];
  AverageGPUMemoryUsagePlot /= GPUMemoryUsagePlot.size();

  CurrentPlotIndex = (CurrentPlotIndex + 1) % PlotWindowWidth;

  // Stats
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

      if (ImGui::Button("Open Lights viewer"))
      {
        LightToolActive = true;
        StatsToolActive = false;
      }

      ImGui::Text("Average FPS = %.4g", Engine->GetFPS());
      ImGui::Text("Average Render Time on CPU= %.4g ms", AverageCPURenderTimePlot);
      ImGui::Text("Average Time for Upload CL = %.4g ms", AverageCPUUpdateTimePlot);
      ImGui::Text("Average Time for Render CL = %.4g ms", AverageCPUFrameRenderTimePlot);
      ImGui::Text("Average Device Render Time = %f ms", AverageDeviceRenderTimePlot);
      ImGui::Text("Average GPU Usage = %f MB", AverageGPUMemoryUsagePlot);
      ImGui::Text("Max GPU budget = %f MB", MaxGPUMemoryPlot);
      ImGui::Text("Objects amount = %zd", Engine->ObjectSystem->CPUPool.size());
      ImGui::Text("Geometry amount = %zd", Engine->GeometrySystem->CPUPool.size());
      ImGui::Text("Materials amount = %zd", Engine->MaterialsSystem->CPUData.size());
      ImGui::Text("Max Textures amount = %zd", gdr::MAX_TEXTURE_AMOUNT);
      ImGui::Text("Max Cube textures amount = %zd", gdr::MAX_CUBE_TEXTURE_AMOUNT);
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
      ImGui::Checkbox("Present Mode", &PresentMode);

      ImGui::DragFloat3("Camera Position", const_cast<float *>(&Engine->PlayerCamera.GetPos().X), 0.1);
      ImGui::DragFloat3("Camera Direction", const_cast<float*>(&Engine->PlayerCamera.GetDir().X), 0.1);
      ImGui::DragFloat3("Camera Up", const_cast<float*>(&Engine->PlayerCamera.GetUp().X), 0.1);
      ImGui::DragFloat("Exposure", &Engine->GlobalsSystem->CPUData.SceneExposure, 0.1);
      if (ImGui::Button("Apply Camera Transform"))
        Engine->PlayerCamera.SetView(Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetDir() + Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetUp());
      ImGui::End();
    });


  if (Engine->KeysClick[VK_OEM_3]) // TILDE
  {
    StatsToolActive = true;
  }
}