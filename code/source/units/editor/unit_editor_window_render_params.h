#pragma once
#include "unit_editor.h"
#include "../unit_base.h"

class unit_editor_window_render_params : public gdr::unit_base
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

      if (ImGui::Begin("Render params", &isShow))
      {
        ImGui::Checkbox("Move updates in separate cmdList", &Engine->Params.IsUploadEveryFrame);
        ImGui::Checkbox("Indirect Render", &Engine->Params.IsIndirect);
        ImGui::Checkbox("Frustum Culling", &Engine->Params.IsFrustumCulling);
        if (Engine->Params.IsIndirect)
          ImGui::Checkbox("Occlusion Culling", &Engine->Params.IsOccusionCulling);
        ImGui::Checkbox("Lock view", &Engine->Params.IsViewLocked);
        ImGui::Checkbox("Show AABB", &Engine->Params.IsShowAABB);
        ImGui::Checkbox("Show Hierarcy", &Engine->Params.IsShowHier);
        ImGui::Checkbox("Tonemapping", &Engine->Params.IsTonemapping);
        ImGui::DragFloat("Scene Exposure", &Engine->Params.SceneExposure, 0.1f);
        ImGui::Checkbox("IBL", &Engine->Params.IsIBL);
        ImGui::Checkbox("Transparency", &Engine->Params.IsTransparent);
        if (Engine->Params.IsTransparent)
          ImGui::Checkbox("Debug transparency", &Engine->Params.IsDebugOIT);
        ImGui::Checkbox("FXAA", &Engine->Params.IsFXAA);
        bool pause = Engine->GetPause();
        if (ImGui::Checkbox("Pause", &pause))
        {
          Engine->SetPause(pause);
        }

        if (ImGui::Button("Assert now"))
          GDR_ASSERT(0);
      }
      ImGui::End();
      FloatVars["Show"] = isShow;
      });
  }

  std::string GetName(void)
  {
    return "unit_editor_window_render_params";
  }
};