#pragma once
#pragma once
#include "../unit_base.h"

class unit_render_params : public gdr::unit_base
{
private:
  bool MainWindow;
public:
    void Initialize()
    {
    }

    void Response(void)
    {
      Engine->AddLambdaForIMGUI(
        [&]()
        {
          ImGui::Begin("Render params", &MainWindow, ImGuiWindowFlags_AlwaysAutoResize);
          ImGui::Text("Average FPS = %.4g", Engine->GetFPS());

          DXGI_QUERY_VIDEO_MEMORY_INFO gpu_info;
          ((IDXGIAdapter3*)Engine->GetDevice().GetAdapter())->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &gpu_info);
          float CurrentGPUMemory = gpu_info.CurrentUsage / 1024.0f / 1024.0f;
          float MaxGPUMemoryPlot = gpu_info.Budget / 1024.0f / 1024.0f;

          ImGui::Text("Current GPU Usage = %f MB", CurrentGPUMemory);
          ImGui::Text("Max GPU budget = %f MB", MaxGPUMemoryPlot);
          ImGui::Text("Objects allocated = %zd", Engine->DrawCommandsSystem->AllocatedSize());
          ImGui::Text("Materials allocated = %zd", Engine->MaterialsSystem->AllocatedSize());
          ImGui::Text("Max Textures amount = %zd", Engine->CreationParams.MaxTextureAmount);
          ImGui::Checkbox("Indirect Render", &Engine->Params.IsIndirect);
          ImGui::Checkbox("Frustum Culling", &Engine->Params.IsFrustumCulling);
          if (Engine->Params.IsIndirect)
            ImGui::Checkbox("Occlusion Culling", &Engine->Params.IsOccusionCulling);
          ImGui::Checkbox("Lock view", &Engine->Params.IsViewLocked);
          ImGui::Checkbox("Show AABB", &Engine->Params.IsShowAABB);
          ImGui::Checkbox("Tonemapping", &Engine->Params.IsTonemapping);
          bool pause = Engine->GetPause();
          if (ImGui::Checkbox("Pause", &pause))
          {
            Engine->SetPause(pause);
          }
          
          ImGui::DragFloat3("Camera Position", const_cast<float*>(&Engine->PlayerCamera.GetPos().X), 0.1f);
          ImGui::DragFloat3("Camera Direction", const_cast<float*>(&Engine->PlayerCamera.GetDir().X), 0.1f);
          ImGui::DragFloat3("Camera Up", const_cast<float*>(&Engine->PlayerCamera.GetUp().X), 0.1f);
          if (ImGui::Button("Apply Camera Transform"))
            Engine->PlayerCamera.SetView(Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetDir() + Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetUp());
          if (ImGui::Button("Assert now"))
            GDR_ASSERT(0);
          ImGui::End();
        });
    }

    std::string GetName(void)
    {
        return "unit_render_params";
    }

    ~unit_render_params(void)
    {
    }
};