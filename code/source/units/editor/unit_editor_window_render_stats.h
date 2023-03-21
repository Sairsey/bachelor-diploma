#pragma once
#include "unit_editor.h"
#include "../unit_base.h"

class unit_editor_window_render_stats : public gdr::unit_base
{
public:
  void Initialize(void)
  {
    FloatVars["Show"] = 1;
  }

  void Response(void) override
  {
    if (FloatVars["Show"] == 0 || ParentUnit->FloatVars["Show"] == 0)
      return;

    Engine->AddLambdaForIMGUI([&]() {
      bool isShow = FloatVars["Show"];
      if (ImGui::Begin("Render stats", &isShow))
      {
        ImGui::Text("Average FPS = %.4g", Engine->GetFPS());
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Root", "Device Frame Time %f ms(%f FPS)", Engine->DeviceFrameCounter.GetMSec(), 1000 / Engine->DeviceFrameCounter.GetMSec()))
        {
          for (int i = 0; i < Engine->Passes.size(); i++)
          {
            std::string label = Engine->Passes[i]->GetName() + " " + std::to_string(Engine->Passes[i]->DeviceTimeCounter.GetMSec()) + " ms";
            if (ImGui::TreeNode((void*)(intptr_t)i, label.c_str(), i))
            {
              ImGui::TreePop();
            }
          }
          ImGui::TreePop();
        }

        DXGI_QUERY_VIDEO_MEMORY_INFO gpu_info;
        ((IDXGIAdapter3*)Engine->GetDevice().GetAdapter())->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &gpu_info);
        float CurrentGPUMemory = gpu_info.CurrentUsage / 1024.0f / 1024.0f;
        float MaxGPUMemoryPlot = gpu_info.Budget / 1024.0f / 1024.0f;

        ImGui::Text("Current GPU Usage = %f MB", CurrentGPUMemory);
        ImGui::Text("Max GPU budget = %f MB", MaxGPUMemoryPlot);
        ImGui::Text("Draw commands allocated = %zd", Engine->DrawCommandsSystem->AllocatedSize());
        ImGui::Text("Geometries allocated = %zd", Engine->GeometrySystem->AllocatedSize());
        ImGui::Text("Lights allocated = %zd", Engine->LightsSystem->AllocatedSize());
        ImGui::Text("Materials allocated = %zd", Engine->MaterialsSystem->AllocatedSize());
        ImGui::Text("BoneMappings allocated = %zd", Engine->BoneMappingSystem->AllocatedSize());
        ImGui::Text("Object Transforms allocated = %zd", Engine->ObjectTransformsSystem->AllocatedSize());
        ImGui::Text("Node Transforms allocated = %zd", Engine->NodeTransformsSystem->AllocatedSize());
        ImGui::Text("Max Textures amount = %zd", Engine->CreationParams.MaxTextureAmount);
      }
      ImGui::End();
      FloatVars["Show"] = isShow;
      });
  }

  std::string GetName(void)
  {
    return "unit_editor_window_render_stats";
  }
};