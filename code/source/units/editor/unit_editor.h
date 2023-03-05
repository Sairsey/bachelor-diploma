#pragma once
#include "../unit_base.h"

class unit_editor : public gdr::unit_base
{
private:
  ImVec2 GameWindowSize;

  bool IsGameWindow = true;
  bool IsHierarchyWindow = true;
  bool IsDemoWindow = false;
  bool IsRenderStats = true;
  bool IsRenderParams = true;
  bool IsEditWindow = true;

  enum editor_type
  {
    none, 
    camera,
    unit,
    resource_index,
    count
  };

  editor_type TypeOfEditor = none;

  
public:
  void Initialize(void)
  {
  }

  void ShowGameWindow(void)
  {
    // Create window with our render
    if (!IsGameWindow)
      return;

    ImGui::Begin("Game", &IsGameWindow);
    // Using a Child allow to fill all the space of the window.
    // It also alows customization
    ImGui::BeginChild("GameRender");
    // Get the size of the child (i.e. the whole draw size of the windows).
    GameWindowSize = ImGui::GetWindowSize();

    D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->RenderTargetsSystem->ShaderResourceViewsGPU;
    true_texture_handle.ptr += (int)(gdr::render_targets_enum::target_frame_final)*Engine->GetDevice().GetSRVDescSize();
    ImGui::Image((ImTextureID)true_texture_handle.ptr, GameWindowSize);

    ImGui::EndChild();
    ImGui::End();
  }

  void ShowHierarchyWindow(void)
  {
    if (!IsHierarchyWindow)
      return;

    ImGui::Begin("Hierarchy", &IsHierarchyWindow);
    if (ImGui::TreeNode("GDR"))
    {
      // add special nodes
      ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | (TypeOfEditor == camera ? ImGuiTreeNodeFlags_Selected : 0));

      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        TypeOfEditor = camera;

      if (ImGui::TreeNode("Units"))
      {
        for (int i = 0; i < Engine->Units.size(); i++)
        {
          ImGui::TreeNodeEx((void*)(intptr_t)i,  ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, Engine->Units[i]->GetName().c_str());
          if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            TypeOfEditor = none;
        }
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }

    ImGui::End();
  }

  void ShowRenderStats(void)
  {
    if (!IsRenderStats)
      return;

    ImGui::Begin("Render stats", &IsRenderStats, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Average FPS = %.4g", Engine->GetFPS());

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
    ImGui::End();
  }

  void ShowRenderParams(void)
  {
    if (!IsRenderParams)
      return;

    ImGui::Begin("Render params", &IsRenderParams, ImGuiWindowFlags_AlwaysAutoResize);
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
    ImGui::End();
  }

  void ShowDemoWindow(void)
  {
    if (!IsDemoWindow)
      return;
    ImGui::ShowDemoWindow(&IsDemoWindow);
  }

  void ShowEditCamera(void)
  {
    ImGui::DragFloat3("Camera Position", const_cast<float*>(&Engine->PlayerCamera.GetPos().X), 0.1f);
    ImGui::DragFloat3("Camera Direction", const_cast<float*>(&Engine->PlayerCamera.GetDir().X), 0.1f);
    ImGui::DragFloat3("Camera Up", const_cast<float*>(&Engine->PlayerCamera.GetUp().X), 0.1f);
    if (ImGui::Button("Apply Camera Transform"))
      Engine->PlayerCamera.SetView(Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetDir() + Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetUp());
  }

  void ShowEditWindow(void)
  {
    if (!IsEditWindow)
      return;

    ImGui::Begin("Editor", &IsEditWindow, ImGuiWindowFlags_AlwaysAutoResize);

    switch (TypeOfEditor)
    {
    case unit_editor::camera:
      ShowEditCamera();
      break;
    case unit_editor::unit:
      break;
    case unit_editor::resource_index:
      break;
    case unit_editor::none:
    case unit_editor::count:
    default:
      break;
    }

    ImGui::End();
  }

  void Response(void)
  {
    Engine->ResizeImgui(max(GameWindowSize.x, 128), max(GameWindowSize.y, 128));
    
    Engine->AddLambdaForIMGUI([&]()
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        
        //Show All widows
        ShowGameWindow();
        ShowHierarchyWindow();
        ShowRenderParams();
        ShowRenderStats();
        ShowDemoWindow();
        ShowEditWindow();

        // show main menu
        if (ImGui::BeginMainMenuBar()) 
        {
          if (ImGui::BeginMenu("View"))
          {
            ImGui::Checkbox("Game", &IsGameWindow);
            ImGui::Checkbox("Hierarchy", &IsHierarchyWindow);
            ImGui::Checkbox("Render params", &IsRenderParams);
            ImGui::Checkbox("Render stats", &IsRenderStats);
            ImGui::Checkbox("Editor", &IsEditWindow);
            ImGui::Checkbox("Demo", &IsDemoWindow);
            ImGui::EndMenu();
          }
          ImGui::EndMainMenuBar();
        }
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