#pragma once
#include "../unit_base.h"

class unit_editor : public gdr::unit_base
{
private:
  ImVec2 wsize;
public:
  void Initialize(void)
  {
  }

  void Response(void)
  {
    static int IsFirst = 0;
    if (IsFirst > 60)
    {
      //Engine->Width = 
      //Engine->Height = ;
      Engine->ResizeImgui(max(wsize.x, 128), max(wsize.y, 128));
    }
    else
      IsFirst++;
    
    Engine->AddLambdaForIMGUI([&]()
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        
        // Create window with our render
        {
          ImGui::Begin("Game");
          // Using a Child allow to fill all the space of the window.
          // It also alows customization
          ImGui::BeginChild("GameRender");
            // Get the size of the child (i.e. the whole draw size of the windows).
            wsize = ImGui::GetWindowSize();
       
            D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->RenderTargetsSystem->ShaderResourceViewsGPU;
            true_texture_handle.ptr += (int)(gdr::render_targets_enum::target_frame_final) * Engine->GetDevice().GetSRVDescSize();
            ImGui::Image((ImTextureID)true_texture_handle.ptr, wsize);
            

          ImGui::EndChild();
          ImGui::End();
        }

        /*
        if (ImGui::BeginMenuBar())
        {
          if (ImGui::BeginMenu("Options"))
          {
            ImGui::EndMenu();
          }
          ImGui::EndMenuBar();
        }*/
    });
  }

  std::string GetName(void)
  {
    return "unit_editor";
  }

  ~unit_editor(void)
  {
  }
};