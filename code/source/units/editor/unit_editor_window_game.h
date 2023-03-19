#pragma once
#include "unit_editor.h"
#include "../unit_base.h"

class unit_editor_window_game : public gdr::unit_base
{
public:
  void Initialize(void)
  {
    FloatVars["Show"] = 1;
  }

  void Response(void) override
  {
    if (FloatVars["Show"] == 0)
      return;

    Engine->AddLambdaForIMGUI([&]() {
        ImGui::Begin("Game", reinterpret_cast<bool *>(&FloatVars["Show"]));
        // Using a Child allow to fill all the space of the window.
        // It also alows customization
        ImGui::BeginChild("GameRender");
        // Get the size of the child (i.e. the whole draw size of the windows).
        ParentUnit->Float2Vars["GameWindowSize"] = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };
        ParentUnit->Float2Vars["GameWindowPos"] = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };

        D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->RenderTargetsSystem->ShaderResourceViewsGPU;
        true_texture_handle.ptr += (int)(gdr::render_targets_enum::target_frame_final)*Engine->GetDevice().GetSRVDescSize();
        ImGui::Image((ImTextureID)true_texture_handle.ptr, ImGui::GetWindowSize());

        ImGui::EndChild();
        ImGui::End();
      });
  }

  std::string GetName(void)
  {
    return "unit_editor_window_game";
  }
};