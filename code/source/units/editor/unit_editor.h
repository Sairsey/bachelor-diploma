#pragma once
#include "../unit_base.h"
#include <json.hpp>
#include <fstream>


// macro to draw a tree of specific resource
#define ResourceTree(index_type, engine_system, string_name, string_many_name, start_index, is_add, add_lambda) \
  {                                                                                                        \
    ImGuiTreeNodeFlags root_flags = ImGuiTreeNodeFlags_AllowItemOverlap;                                   \
    if (ChoosedElement.type == index_type)                                                                 \
      root_flags |= ImGuiTreeNodeFlags_DefaultOpen;                                                        \
    bool is_open = ImGui::TreeNodeEx((string_many_name " (" + std::to_string(engine_system->AllocatedSize() - start_index) + ")").c_str(), root_flags);\
    if (is_add)                                                                                            \
    {                                                                                                      \
      ImGui::SameLine();                                                                                   \
      if (ImGui::SmallButton("+")) add_lambda();                                                           \
    }                                                                                                      \
    if (is_open)                                                                                           \
    {                                                                                                      \
      for (unsigned i = start_index; i < engine_system->AllocatedSize(); i++)                              \
      {                                                                                                    \
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;          \
        if (ChoosedElement.type == index_type && ChoosedElement.value == i)                                \
          flags |= ImGuiTreeNodeFlags_Selected;                                                            \
        std::string name = string_name " #" + std::to_string(i) + " (";                                    \
        if (engine_system->IsExist(i))                                                                     \
          name += "Alive)";                                                                                \
        else                                                                                               \
          name += "Not alive)";                                                                            \
          ImGui::TreeNodeEx(name.c_str(), flags);                                                          \
          if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())                                       \
          {                                                                                                \
            TypeOfEditor = resource_index;                                                                 \
            ChoosedElement.type = index_type;                                                              \
            ChoosedElement.value = i;                                                                      \
          }                                                                                                \
      }                                                                                                    \
      ImGui::TreePop();                                                                                    \
    }                                                                                                      \
  }

class unit_editor : public gdr::unit_base
{
private:
  ImVec2 GameWindowSize;

  bool ShowEditor = true;
  bool ClearScene = false;

  bool IsGameWindow = true;
  bool IsHierarchyWindow = true;
  bool IsDemoWindow = false;
  bool IsRenderStats = true;
  bool IsRenderParams = true;
  bool IsEditWindow = true;
  bool IsResourcesWindow = true;

  enum editor_type
  {
    none, 
    camera,
    unit,
    resource_index,
    count
  };

  editor_type TypeOfEditor = none;

  gdr_index ChoosedElement = NONE_INDEX;

  gdr_index PointLightObject;
  gdr_index DirLightObject;
  gdr_index SpotLightObject;
  gdr_index AxisObject;

  ImGui::FileBrowser modelFileDialog;
  ImGui::FileBrowser saveSceneFileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
  ImGui::FileBrowser loadSceneFileDialog;

  std::vector<std::pair<gdr_index, gdr_index>> Models;
  std::vector<gdr_index> Lights;

  std::string AskedModel = "";
  std::string AskedLoadScene = "";
  std::string AskedSaveScene = "";
public:
  void Initialize(void)
  {
    auto import_data_sphere = gdr::ImportModelFromAssimp("bin/models/light_meshes/sphere.obj");
    auto import_data_dir = gdr::ImportModelFromAssimp("bin/models/light_meshes/dir.obj");
    auto import_data_cone = gdr::ImportModelFromAssimp("bin/models/light_meshes/cone.obj");
    auto import_data_axis = gdr::ImportModelFromAssimp("bin/models/light_meshes/axis.obj");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "load light objects");
    PointLightObject = Engine->ModelsManager->Add(import_data_sphere);
    DirLightObject = Engine->ModelsManager->Add(import_data_dir);
    SpotLightObject = Engine->ModelsManager->Add(import_data_cone);
    AxisObject = Engine->ModelsManager->Add(import_data_axis);
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();

    modelFileDialog.SetTitle("Choose a model...");
    modelFileDialog.SetTypeFilters({ ".obj", ".fbx", ".glb" });

    saveSceneFileDialog.SetTitle("Save scene to...");
    saveSceneFileDialog.SetTypeFilters({".json"});

    loadSceneFileDialog.SetTitle("Load scene from...");
    loadSceneFileDialog.SetTypeFilters({ ".json" });
  }

  void LoadModel(std::string path)
  {
      auto import_data = gdr::ImportModelFromAssimp(path);

      if (!import_data.IsEmpty())
      {
          Engine->GetDevice().WaitAllUploadLists();
          Engine->GetDevice().WaitGPUIdle();
          Engine->GetDevice().ResizeUpdateBuffer(false);
          ID3D12GraphicsCommandList* commandList;
          Engine->GetDevice().BeginUploadCommandList(&commandList);
          PROFILE_BEGIN(commandList, charToWString(path.c_str()).c_str());
          Models.push_back(std::make_pair(Engine->ModelsManager->Add(import_data), Engine->AnimationManager->Add(import_data)));
          PROFILE_END(commandList);
          Engine->GetDevice().CloseUploadCommandList();
          Engine->GetDevice().WaitAllUploadLists();
          Engine->GetDevice().WaitGPUIdle();
          Engine->GetDevice().ResizeUpdateBuffer(true);
      }
  }

  void LoadScene(std::string path)
  {
    std::ifstream i(path);
    nlohmann::json data;
    i >> data;

    for (int i = 0; i < data["models"].size(); i++)
    {
      nlohmann::json sub_data = data["models"][i];

      LoadModel(sub_data["path"]);
      gdr_index modelIndex = Models[Models.size() - 1].first;


      mth::vec3f pos, scale;
      mth::vec4f rot;

      pos.X = sub_data["transform"]["pos"][0];
      pos.Y = sub_data["transform"]["pos"][1];
      pos.Z = sub_data["transform"]["pos"][2];

      rot.X = sub_data["transform"]["rot"][0];
      rot.Y = sub_data["transform"]["rot"][1];
      rot.Z = sub_data["transform"]["rot"][2];
      rot.W = sub_data["transform"]["rot"][3];

      scale.X = sub_data["transform"]["scale"][0];
      scale.Y = sub_data["transform"]["scale"][1];
      scale.Z = sub_data["transform"]["scale"][2];

      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(modelIndex).Render.RootTransform).Transform = mth::matr4f::BuildTransform(scale,rot, pos);
    }

    for (int i = 0; i < data["lights"].size(); i++)
    {
      nlohmann::json sub_data = data["lights"][i];
      gdr_index lightIndex = Engine->LightsSystem->Add();
      Lights.push_back(lightIndex);

      Engine->LightsSystem->GetEditable(lightIndex).LightSourceType = sub_data["type"];
      Engine->LightsSystem->GetEditable(lightIndex).ConstantAttenuation = sub_data["attenuation"]["const"];
      Engine->LightsSystem->GetEditable(lightIndex).LinearAttenuation = sub_data["attenuation"]["linear"];
      Engine->LightsSystem->GetEditable(lightIndex).QuadricAttenuation = sub_data["attenuation"]["quad"];

      Engine->LightsSystem->GetEditable(lightIndex).Color = {sub_data["color"][0], sub_data["color"][1], sub_data["color"][2]};

      Engine->LightsSystem->GetEditable(lightIndex).ObjectTransformIndex = Engine->ObjectTransformsSystem->Add();


      mth::vec3f pos = { 0, 0, 0 }, scale = { 1, 1, 1 };
      mth::vec4f rot = { 0, 0, 0, 1 };

      pos.X = sub_data["transform"]["pos"][0];
      pos.Y = sub_data["transform"]["pos"][1];
      pos.Z = sub_data["transform"]["pos"][2];

      rot.X = sub_data["transform"]["rot"][0];
      rot.Y = sub_data["transform"]["rot"][1];
      rot.Z = sub_data["transform"]["rot"][2];
      rot.W = sub_data["transform"]["rot"][3];

      scale.X = sub_data["transform"]["scale"][0];
      scale.Y = sub_data["transform"]["scale"][1];
      scale.Z = sub_data["transform"]["scale"][2];

      Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->Get(lightIndex).ObjectTransformIndex).Transform = mth::matr4f::BuildTransform(scale, rot, pos);

      Engine->LightsSystem->GetEditable(lightIndex).AngleInnerCone = sub_data["inner_cone"];
      Engine->LightsSystem->GetEditable(lightIndex).AngleOuterCone = sub_data["outer_cone"];
      Engine->LightsSystem->GetEditable(lightIndex).ShadowMapOffset = sub_data["shadow_offset"];

      if (sub_data["is_shadow"].get<bool>())
      {
        Engine->LightsSystem->GetEditable(lightIndex).ShadowMapIndex = Engine->ShadowMapsSystem->Add(sub_data["shadow_size"]["w"], sub_data["shadow_size"]["h"]);
      }
    }
  }

  void SaveScene(std::string path)
  {
    if (path.substr(max(0, path.length() - 5)) != ".json")
      path+= ".json";

    nlohmann::json data;
    data["models"] = {};
    for (int i = 0; i < Models.size(); i++)
    {
      nlohmann::json sub_data;

      sub_data["path"] = Engine->ModelsManager->Get(Models[i].first).Name;

      mth::vec3f pos, scale;
      mth::vec4f rot;

      Engine->ObjectTransformsSystem->Get(Engine->ModelsManager->Get(Models[i].first).Render.RootTransform).Transform.Decompose(pos, rot, scale);

      sub_data["transform"] = {{"pos", {pos.X, pos.Y, pos.Z}}, {"rot", {rot.X, rot.Y, rot.Z, rot.W}}, {"scale", {scale.X, scale.Y, scale.Z}}};

      data["models"].push_back(sub_data);
    }

    data["lights"] = {};
    for (int i = 0; i < Lights.size(); i++)
    {
      nlohmann::json sub_data;

      sub_data["type"] = Engine->LightsSystem->Get(Lights[i]).LightSourceType;
      sub_data["attenuation"] = {
        {"const", Engine->LightsSystem->Get(Lights[i]).ConstantAttenuation},
        {"linear", Engine->LightsSystem->Get(Lights[i]).LinearAttenuation},
        {"quad", Engine->LightsSystem->Get(Lights[i]).QuadricAttenuation}
        };

      sub_data["color"] = { Engine->LightsSystem->Get(Lights[i]).Color.R, Engine->LightsSystem->Get(Lights[i]).Color.G, Engine->LightsSystem->Get(Lights[i]).Color.B};

      mth::vec3f pos = {0, 0, 0}, scale = {1, 1, 1};
      mth::vec4f rot = {0, 0, 0, 1};

      if (Engine->LightsSystem->Get(Lights[i]).ObjectTransformIndex != NONE_INDEX)
        Engine->ObjectTransformsSystem->Get(Engine->LightsSystem->Get(Lights[i]).ObjectTransformIndex).Transform.Decompose(pos, rot, scale);

      sub_data["transform"] = { {"pos", {pos.X, pos.Y, pos.Z}}, {"rot", {rot.X, rot.Y, rot.Z, rot.W}}, {"scale", {scale.X, scale.Y, scale.Z}} };

      sub_data["inner_cone"] = Engine->LightsSystem->Get(Lights[i]).AngleInnerCone;
      sub_data["outer_cone"] = Engine->LightsSystem->Get(Lights[i]).AngleOuterCone;

      sub_data["shadow_offset"] = Engine->LightsSystem->Get(Lights[i]).ShadowMapOffset;

      sub_data["is_shadow"] = Engine->LightsSystem->Get(Lights[i]).ShadowMapIndex != NONE_INDEX;
      if (Engine->LightsSystem->Get(Lights[i]).ShadowMapIndex != NONE_INDEX)
        sub_data["shadow_size"] = {
          {"w",  Engine->ShadowMapsSystem->Get(Engine->LightsSystem->Get(Lights[i]).ShadowMapIndex).W},
          {"h", Engine->ShadowMapsSystem->Get(Engine->LightsSystem->Get(Lights[i]).ShadowMapIndex).H}};

      data["lights"].push_back(sub_data);
    }

    std::ofstream o(path);
    o << std::setw(4) << data << std::endl;
  }

  void Clear(void)
  {
    for (int i = 0; i < Models.size(); i++)
    {
      Engine->ModelsManager->Remove(Models[i].first);
      Engine->AnimationManager->Remove(Models[i].second);
    }
    for (int i = 0; i < Lights.size(); i++)
    {
      Engine->LightsSystem->Remove(Lights[i]);
    }
    Models.clear();
    Lights.clear();
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
    if (ImGui::TreeNodeEx("GDR", ImGuiTreeNodeFlags_DefaultOpen))
    {
      // add special nodes
      ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | (TypeOfEditor == camera ? ImGuiTreeNodeFlags_Selected : 0));

      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        TypeOfEditor = camera;

      if (ImGui::TreeNodeEx("Units", ImGuiTreeNodeFlags_DefaultOpen))
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

  void ShowResourcesWindow(void)
  {
    if (!IsResourcesWindow)
      return;

    ImGui::Begin("Resources", &IsResourcesWindow);
    if (ImGui::TreeNodeEx("GDR", ImGuiTreeNodeFlags_DefaultOpen))
    {
      ResourceTree(gdr_index_types::model, Engine->ModelsManager, "Model", "Models", 0, true, [&]() {modelFileDialog.Open(); });
      ResourceTree(gdr_index_types::animation, Engine->AnimationManager, "Animation", "Animations", 0, false, [&]() {});
      ResourceTree(gdr_index_types::physic_body, Engine->PhysicsManager, "Body", "Physics", 1, false, [&]() {});
      ResourceTree(gdr_index_types::bone_mapping, Engine->BoneMappingSystem, "Mapping", "Bone mappings", 1, false, [&]() {});
      ResourceTree(gdr_index_types::draw_command, Engine->DrawCommandsSystem, "Command", "Draw commands", 1, false, [&]() {});
      ResourceTree(gdr_index_types::geometry, Engine->GeometrySystem, "Geometry", "Geometries", 0, false, [&]() {});
      ResourceTree(gdr_index_types::light, Engine->LightsSystem, "Light", "Lights", 1, true, [&]() { Lights.push_back(Engine->LightsSystem->Add()); });
      ResourceTree(gdr_index_types::material, Engine->MaterialsSystem, "Material", "Materials", 1, false, [&]() {});
      ResourceTree(gdr_index_types::shadow_map, Engine->ShadowMapsSystem, "Shadow Map", "Shadow Maps", 0, false, [&]() {});
      ResourceTree(gdr_index_types::texture, Engine->TexturesSystem, "Texture", "Textures", 0, false, [&]() {});
      ResourceTree(gdr_index_types::cube_texture, Engine->CubeTexturesSystem, "Cube texture", "Cube Textures", 0, false, [&]() {});
      ResourceTree(gdr_index_types::object_transform, Engine->ObjectTransformsSystem, "Transform", "Object Transforms", 1, false, [&]() {});
      ResourceTree(gdr_index_types::node_transform, Engine->NodeTransformsSystem, "Transform", "Node Transforms", 1, false, [&]() {});

      ImGui::TreePop();
    }

    ImGui::End();
  }

  void ShowRenderStats(void)
  {
    if (!IsRenderStats)
      return;

    ImGui::Begin("Render stats", &IsRenderStats, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SetNextItemOpen(true, ImGuiCond_Once); 
    ImGui::Text("Average FPS = %.4g", Engine->GetFPS());
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
    ImGui::Text("Camera Editor");
    ImGui::DragFloat3("Camera Position", const_cast<float*>(&Engine->PlayerCamera.GetPos().X), 0.1f);
    ImGui::DragFloat3("Camera Direction", const_cast<float*>(&Engine->PlayerCamera.GetDir().X), 0.1f);
    ImGui::DragFloat3("Camera Up", const_cast<float*>(&Engine->PlayerCamera.GetUp().X), 0.1f);
    if (ImGui::Button("Apply Camera Transform"))
      Engine->PlayerCamera.SetView(Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetDir() + Engine->PlayerCamera.GetPos(), Engine->PlayerCamera.GetUp());
  }

  void ShowEditResourceLight(void)
  {
    ImGui::Text("Light Editor");
    if (Engine->LightsSystem->IsExist(ChoosedElement))
    {
      if (ImGui::Button("Delete"))
      {
        Engine->LightsSystem->Remove(ChoosedElement);
        return;
      }

      const char* items[] = { "Directional" , "Point", "Spot" };
      ImGui::Combo("Type", (int*)&Engine->LightsSystem->Get(ChoosedElement).LightSourceType, items, IM_ARRAYSIZE(items));
      ImGui::Text("Transform index %d", Engine->LightsSystem->Get(ChoosedElement).ObjectTransformIndex);
      
      if (Engine->LightsSystem->Get(ChoosedElement).ObjectTransformIndex != NONE_INDEX)
      {
        mth::vec3f Translate, Rotate, Scale;
        Engine->ObjectTransformsSystem->Get(Engine->LightsSystem->Get(ChoosedElement).ObjectTransformIndex).Transform.Decompose(Translate, Rotate, Scale);

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
            Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->Get(ChoosedElement).ObjectTransformIndex).Transform = result;
          else
            Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->Get(ChoosedElement).ObjectTransformIndex).Transform = mth::matr4f::Identity();
        }
      }
      else if (ImGui::Button("Add"))
        Engine->LightsSystem->GetEditable(ChoosedElement).ObjectTransformIndex = Engine->ObjectTransformsSystem->Add();

      ImGui::ColorEdit3("Color", &Engine->LightsSystem->GetEditable(ChoosedElement).Color[0]);
      ImGui::DragFloat("Red", &Engine->LightsSystem->GetEditable(ChoosedElement).Color[0], 0.1);
      ImGui::DragFloat("Green", &Engine->LightsSystem->GetEditable(ChoosedElement).Color[1], 0.1);
      ImGui::DragFloat("Blue", &Engine->LightsSystem->GetEditable(ChoosedElement).Color[2], 0.1);

      if (Engine->LightsSystem->Get(ChoosedElement).LightSourceType != LIGHT_SOURCE_TYPE_DIRECTIONAL)
      {
        ImGui::Text("Attenuation");
        ImGui::DragFloat("Constant part", &Engine->LightsSystem->GetEditable(ChoosedElement).ConstantAttenuation, 0.1);
        ImGui::DragFloat("Linear part", &Engine->LightsSystem->GetEditable(ChoosedElement).LinearAttenuation, 0.1);
        ImGui::DragFloat("Quadric part", &Engine->LightsSystem->GetEditable(ChoosedElement).QuadricAttenuation, 0.1);
        Engine->LightsSystem->GetEditable(ChoosedElement).ConstantAttenuation =
          max(1, Engine->LightsSystem->Get(ChoosedElement).ConstantAttenuation);
      }

      if (Engine->LightsSystem->Get(ChoosedElement).LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
      {
        Engine->LightsSystem->GetEditable(ChoosedElement).AngleInnerCone *= MTH_R2D;
        Engine->LightsSystem->GetEditable(ChoosedElement).AngleOuterCone *= MTH_R2D;
        ImGui::DragFloat("Inner cone angle", &Engine->LightsSystem->GetEditable(ChoosedElement).AngleInnerCone, 0.1, 1);
        ImGui::DragFloat("Outer cone angle", &Engine->LightsSystem->GetEditable(ChoosedElement).AngleOuterCone, 0.1, Engine->LightsSystem->GetEditable(ChoosedElement).AngleInnerCone);
        Engine->LightsSystem->GetEditable(ChoosedElement).AngleInnerCone = max(5, Engine->LightsSystem->Get(ChoosedElement).AngleInnerCone);
        Engine->LightsSystem->GetEditable(ChoosedElement).AngleOuterCone = max(Engine->LightsSystem->Get(ChoosedElement).AngleInnerCone, Engine->LightsSystem->Get(ChoosedElement).AngleOuterCone);
        Engine->LightsSystem->GetEditable(ChoosedElement).AngleInnerCone *= MTH_D2R;
        Engine->LightsSystem->GetEditable(ChoosedElement).AngleOuterCone *= MTH_D2R;
      }
      else if (Engine->LightsSystem->Get(ChoosedElement).LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
      {
          ImGui::DragFloat("Shadow Map Size", &Engine->LightsSystem->GetEditable(ChoosedElement).AngleInnerCone, 0.1, 1);
      }

      if (Engine->LightsSystem->Get(ChoosedElement).LightSourceType != LIGHT_SOURCE_TYPE_POINT)
      {
        if (Engine->LightsSystem->Get(ChoosedElement).ShadowMapIndex == NONE_INDEX)
        {
          static int W = 128, H = 128;
          ImGui::Text("Shadow map");
          ImGui::DragInt("W", &W);
          ImGui::DragInt("H", &H);
          if (ImGui::Button("Add"))
            Engine->LightsSystem->GetEditable(ChoosedElement).ShadowMapIndex = Engine->ShadowMapsSystem->Add(W, H);
        }
        else
        {
          int ShadowMapIndex = Engine->LightsSystem->Get(ChoosedElement).ShadowMapIndex;
          auto& el = Engine->ShadowMapsSystem->Get(ShadowMapIndex);

          ImGui::Text("Shadow map index %d", ShadowMapIndex);
          if (ImGui::Button("Delete shadow map"))
          {
            Engine->ShadowMapsSystem->Remove(ShadowMapIndex);
            Engine->LightsSystem->GetEditable(ChoosedElement).ShadowMapIndex = NONE_INDEX;
          }

          ImGui::Text("Width : %d", el.W);
          ImGui::Text("Heigth: %d", el.H);
          ImGui::DragFloat("Offset", &Engine->LightsSystem->GetEditable(ChoosedElement).ShadowMapOffset);

          D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->ShadowMapsSystem->ShadowMapTableGPU;
          true_texture_handle.ptr += ShadowMapIndex * Engine->GetDevice().GetSRVDescSize();
          ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2((float)128, (float)128));
        }
      }
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceObjectTransform(void)
  {
    ImGui::Text("Object Transform Editor");
    if (Engine->ObjectTransformsSystem->IsExist(ChoosedElement))
    {
      mth::vec3f Translate, Rotate, Scale;
      Engine->ObjectTransformsSystem->Get(ChoosedElement).Transform.Decompose(Translate, Rotate, Scale);

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
          Engine->ObjectTransformsSystem->GetEditable(ChoosedElement).Transform = result;
        else
          Engine->ObjectTransformsSystem->GetEditable(ChoosedElement).Transform = mth::matr4f::Identity();
      }

      mth::vec3f minAABB = Engine->ObjectTransformsSystem->Get(ChoosedElement).minAABB;
      mth::vec3f maxAABB = Engine->ObjectTransformsSystem->Get(ChoosedElement).maxAABB;

      if (ImGui::DragFloat3("minAABB", &minAABB.X, 0.1))
      {
        Engine->ObjectTransformsSystem->GetEditable(ChoosedElement).minAABB = minAABB;
      }

      if (ImGui::DragFloat3("maxAABB", &maxAABB.X, 0.1))
      {
        Engine->ObjectTransformsSystem->GetEditable(ChoosedElement).maxAABB = maxAABB;
      }
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }
  
  void ShowEditResourceNodeTransform(void)
  {
    ImGui::Text("Node Transform Editor");
    if (Engine->NodeTransformsSystem->IsExist(ChoosedElement))
    {
      ImGui::Text("Parent index %d", Engine->NodeTransformsSystem->Get(ChoosedElement).ParentIndex);
      if (Engine->NodeTransformsSystem->Get(ChoosedElement).ParentIndex != NONE_INDEX)
      {
        ImGui::SameLine();
        if (ImGui::Button("Go to parent"))
        {
            ChoosedElement = Engine->NodeTransformsSystem->Get(ChoosedElement).ParentIndex;
            return;
        }
      }

      ImGui::Text("First Child index %d", Engine->NodeTransformsSystem->Get(ChoosedElement).ChildIndex);
      if (Engine->NodeTransformsSystem->Get(ChoosedElement).ChildIndex != NONE_INDEX)
      {
        ImGui::SameLine();
        if (ImGui::Button("Go to first children"))
        {
            ChoosedElement = Engine->NodeTransformsSystem->Get(ChoosedElement).ChildIndex;
            return;
        }
      }

      ImGui::Text("Next sibling index %d", Engine->NodeTransformsSystem->Get(ChoosedElement).NextIndex);
      if (Engine->NodeTransformsSystem->Get(ChoosedElement).NextIndex != NONE_INDEX)
      {
        ImGui::SameLine();
        if (ImGui::Button("Go to next sibling"))
        {
            ChoosedElement = Engine->NodeTransformsSystem->Get(ChoosedElement).NextIndex;
            return;
        }
      }

      ImGui::Text("Local Transform");
      
      mth::vec3f Translate, Rotate, Scale;
      Engine->NodeTransformsSystem->Get(ChoosedElement).LocalTransform.Decompose(Translate, Rotate, Scale);

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
          Engine->NodeTransformsSystem->GetEditable(ChoosedElement).LocalTransform = result;
        else
          Engine->NodeTransformsSystem->GetEditable(ChoosedElement).LocalTransform = mth::matr4f::Identity();
      }

      ImGui::Text("Global Transform");
      Engine->NodeTransformsSystem->Get(ChoosedElement).GlobalTransform.Decompose(Translate, Rotate, Scale);

      ImGui::Text("Translation: %f %f %f", Translate.X, Translate.Y, Translate.Z);
      ImGui::Text("Rotation: %f %f %f", Rotate.X, Rotate.Y, Rotate.Z);
      ImGui::Text("Scale: %f %f %f", Scale.X, Scale.Y, Scale.Z);

      ImGui::Text("Bone offset");
      Engine->NodeTransformsSystem->Get(ChoosedElement).BoneOffset.Decompose(Translate, Rotate, Scale);

      ImGui::Text("Translation: %f %f %f", Translate.X, Translate.Y, Translate.Z);
      ImGui::Text("Rotation: %f %f %f", Rotate.X, Rotate.Y, Rotate.Z);
      ImGui::Text("Scale: %f %f %f", Scale.X, Scale.Y, Scale.Z);
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceTexture(void)
  {
    ImGui::Text("Texture Editor");
    if (Engine->TexturesSystem->IsExist(ChoosedElement))
    {
      ImGui::Text("Name: %s", Engine->TexturesSystem->Get(ChoosedElement).Name.c_str());
      ImGui::Text("W: %d", Engine->TexturesSystem->Get(ChoosedElement).W);
      ImGui::Text("H: %d", Engine->TexturesSystem->Get(ChoosedElement).H);
      ImGui::Text("Mips: %d", Engine->TexturesSystem->Get(ChoosedElement).NumOfMips);
      ImGui::Text("Transparent: %s", Engine->TexturesSystem->Get(ChoosedElement).IsTransparent ? "TRUE" : "FALSE");
      
      D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->TexturesSystem->TextureTableGPU;
      true_texture_handle.ptr += ChoosedElement * Engine->GetDevice().GetSRVDescSize();

      float aspect = Engine->TexturesSystem->Get(ChoosedElement).H / Engine->TexturesSystem->Get(ChoosedElement).W;

      ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2(128.0f, 128.0f * aspect));      
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceCubeTexture(void)
  {
    ImGui::Text("CubeTexture Editor");
    if (Engine->CubeTexturesSystem->IsExist(ChoosedElement))
    {
      ImGui::Text("Name: %s", Engine->CubeTexturesSystem->Get(ChoosedElement).Name.c_str());
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceShadowMap(void)
  {
    ImGui::Text("Shadow Map Editor");
    if (Engine->ShadowMapsSystem->IsExist(ChoosedElement))
    {
      ImGui::Text("W: %d", Engine->ShadowMapsSystem->Get(ChoosedElement).W);
      ImGui::Text("H: %d", Engine->ShadowMapsSystem->Get(ChoosedElement).H);
      
      D3D12_GPU_DESCRIPTOR_HANDLE true_texture_handle = Engine->ShadowMapsSystem->ShadowMapTableGPU;
      true_texture_handle.ptr += ChoosedElement * Engine->GetDevice().GetSRVDescSize();

      float aspect = Engine->ShadowMapsSystem->Get(ChoosedElement).H / Engine->ShadowMapsSystem->Get(ChoosedElement).W;

      ImGui::Image((ImTextureID)true_texture_handle.ptr, ImVec2(128.0f, 128.0f * aspect));
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceBoneMapping(void)
  {
    ImGui::Text("Bone Mapping Editor");
    if (Engine->BoneMappingSystem->IsExist(ChoosedElement))
    {
      ImGui::BeginTable("Values", 2, ImGuiTableFlags_Borders);
      ImGui::TableNextColumn();
      ImGui::Text("Geom index");
      ImGui::TableNextColumn();
      ImGui::Text("Node index");
      ImGui::TableNextRow();

      for (int i = 0; i < MAX_BONE_PER_MESH; i++)
      {
        ImGui::TableNextColumn();
        ImGui::Text("%d", i);
        ImGui::TableNextColumn();
        ImGui::Text("%d", Engine->BoneMappingSystem->Get(ChoosedElement).BoneMapping[i]);
        ImGui::TableNextRow();
      }
      ImGui::EndTable();
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceMaterial(void)
  {
    ImGui::Text("Material Editor");
    if (Engine->MaterialsSystem->IsExist(ChoosedElement))
    {
      const char* items[] = { "Color" , "Phong", "PBR Metal/Rough", "PBR Shiness/Glossiness"};
      ImGui::Combo("Type", (int*)&Engine->MaterialsSystem->Get(ChoosedElement).ShadeType, items, IM_ARRAYSIZE(items));

      if (Engine->MaterialsSystem->Get(ChoosedElement).ShadeType == MATERIAL_SHADER_COLOR)
      {
        GDRGPUMaterial &el = Engine->MaterialsSystem->GetEditable(ChoosedElement);
        ImGui::ColorEdit3("Color", &GDRGPUMaterialColorGetColor(el).X);
        ImGui::Text("Opacity %g", GDRGPUMaterialColorGetOpacity(el));
        ImGui::Text("Color texture index %d", GDRGPUMaterialColorGetColorMapIndex(el));
        if (GDRGPUMaterialColorGetColorMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to color texture"))
          {
            ChoosedElement = GDRGPUMaterialColorGetColorMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }
      }
      
      if (Engine->MaterialsSystem->Get(ChoosedElement).ShadeType == MATERIAL_SHADER_PHONG)
      {
        GDRGPUMaterial& el = Engine->MaterialsSystem->GetEditable(ChoosedElement);
        ImGui::ColorEdit3("Ambient", &GDRGPUMaterialPhongGetAmbient(el).X);
        ImGui::Text("Ambient texture index %d", GDRGPUMaterialPhongGetAmbientMapIndex(el));
        if (GDRGPUMaterialPhongGetAmbientMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to ambient texture"))
          {
            ChoosedElement = GDRGPUMaterialPhongGetAmbientMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::ColorEdit3("Diffuse", &GDRGPUMaterialPhongGetDiffuse(el).X);
        ImGui::Text("Diffuse texture index %d", GDRGPUMaterialPhongGetDiffuseMapIndex(el));
        if (GDRGPUMaterialPhongGetDiffuseMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to diffuse texture"))
          {
            ChoosedElement = GDRGPUMaterialPhongGetDiffuseMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::ColorEdit3("Specular", &GDRGPUMaterialPhongGetSpecular(el).X);
        ImGui::Text("Specular texture index %d", GDRGPUMaterialPhongGetSpecularMapIndex(el));
        if (GDRGPUMaterialPhongGetSpecularMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to specular texture"))
          {
            ChoosedElement = GDRGPUMaterialPhongGetSpecularMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::DragFloat("Shiness", &GDRGPUMaterialPhongGetShiness(el));

        ImGui::Text("Normal map index %d", GDRGPUMaterialPhongGetNormalMapIndex(el));
        if (GDRGPUMaterialPhongGetNormalMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to normal map"))
          {
            ChoosedElement = GDRGPUMaterialPhongGetNormalMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }
      }

      if (Engine->MaterialsSystem->Get(ChoosedElement).ShadeType == MATERIAL_SHADER_COOKTORRANCE_METALNESS)
      {
        GDRGPUMaterial& el = Engine->MaterialsSystem->GetEditable(ChoosedElement);
        ImGui::Text("Ambient occlusion map index %d", GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to ambient texture"))
          {
            ChoosedElement = GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::ColorEdit3("Albedo", &GDRGPUMaterialCookTorranceGetAlbedo(el).X);
        ImGui::Text("Albedo map index %d", GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to albedo texture"))
          {
            ChoosedElement = GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::Text("Normal map index %d", GDRGPUMaterialCookTorranceGetNormalMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetNormalMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to normal map"))
          {
            ChoosedElement = GDRGPUMaterialCookTorranceGetNormalMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::Text("Opacity %g", GDRGPUMaterialCookTorranceGetOpacity(el));

        ImGui::DragFloat("Roughness", &GDRGPUMaterialCookTorranceGetRoughness(el));
        ImGui::DragFloat("Metalness", &GDRGPUMaterialCookTorranceGetMetalness(el));

        ImGui::Text("Metal/Rough map index %d", GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to metal/rough texture"))
          {
            ChoosedElement = GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }
      }

      if (Engine->MaterialsSystem->Get(ChoosedElement).ShadeType == MATERIAL_SHADER_COOKTORRANCE_SPECULAR)
      {
        GDRGPUMaterial& el = Engine->MaterialsSystem->GetEditable(ChoosedElement);
        ImGui::Text("Ambient occlusion map index %d", GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to ambient texture"))
          {
            ChoosedElement = GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::ColorEdit3("Albedo", &GDRGPUMaterialCookTorranceGetAlbedo(el).X);
        ImGui::Text("Albedo map index %d", GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to albedo texture"))
          {
            ChoosedElement = GDRGPUMaterialCookTorranceGetAlbedoMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::Text("Normal map index %d", GDRGPUMaterialCookTorranceGetNormalMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetNormalMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to normal map"))
          {
            ChoosedElement = GDRGPUMaterialCookTorranceGetNormalMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }

        ImGui::Text("Opacity %g", GDRGPUMaterialCookTorranceGetOpacity(el));

        ImGui::ColorEdit3("Specular", &GDRGPUMaterialCookTorranceGetSpecular(el).R);
        ImGui::DragFloat("Glossiness", &GDRGPUMaterialCookTorranceGetGlossiness(el));

        ImGui::Text("Specular/gloss map index %d", GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(el));
        if (GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(el) != NONE_INDEX)
        {
          ImGui::SameLine();
          if (ImGui::Button("Go to specular/gloss texture"))
          {
            ChoosedElement = GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(el);
            ChoosedElement.type = gdr_index_types::texture;
            return;
          }
        }
      }
    }
    else
    {
      ImGui::Text("Not Alive");
    }
  }

  void ShowEditResourceModel(void)
  {
      ImGui::Text("Model Editor");
      if (Engine->ModelsManager->IsExist(ChoosedElement))
      {
          if (ImGui::Button("Delete Model"))
          {
              for (int i = 0; i < Models.size(); i++)
              {
                  if (Models[i].first == ChoosedElement)
                  {
                      Engine->ModelsManager->Remove(Models[i].first);
                      Engine->AnimationManager->Remove(Models[i].second);
                      Models.erase(std::next(Models.begin(), i));
                      return;
                  }
              }
          }
          ImGui::Text("Name: %s", Engine->ModelsManager->Get(ChoosedElement).Name.c_str());
          ImGui::Text("Root Transform index: %d", Engine->ModelsManager->Get(ChoosedElement).Render.RootTransform);
          if (Engine->ModelsManager->Get(ChoosedElement).Render.RootTransform != NONE_INDEX)
          {
              if (ImGui::Button("Go to Transform"))
              {
                  ChoosedElement = Engine->ModelsManager->Get(ChoosedElement).Render.RootTransform;
                  ChoosedElement.type = gdr_index_types::object_transform;
                  return;
              }
          }

          ImGui::BeginTable("Materials", 2, ImGuiTableFlags_Borders);
          ImGui::TableNextColumn();
          ImGui::Text("Index");
          ImGui::TableNextColumn();
          ImGui::Text("Button to go");
          ImGui::TableNextRow();

          for (int i = 0; i < Engine->ModelsManager->Get(ChoosedElement).Render.Materials.size(); i++)
          {
              ImGui::TableNextColumn();
              ImGui::Text("%d", Engine->ModelsManager->Get(ChoosedElement).Render.Materials[i].value);
              ImGui::TableNextColumn();
              if (Engine->ModelsManager->Get(ChoosedElement).Render.Materials[i].value != NONE_INDEX)
              {
                  if (ImGui::Button((std::string("Go to material ") + std::to_string(Engine->ModelsManager->Get(ChoosedElement).Render.Materials[i].value)).c_str()))
                  {
                      ChoosedElement = Engine->ModelsManager->Get(ChoosedElement).Render.Materials[i];
                      ChoosedElement.type = gdr_index_types::material;
                      ImGui::TableNextRow();
                      ImGui::EndTable(); 
                      return;
                  }
              }
              ImGui::TableNextRow();
          }
          ImGui::EndTable();
      }
      else
      {
          ImGui::Text("Not Alive");
      }
  }

  void ShowEditResource(void)
  {
    ImGui::Text("Resource Editor");
    switch (ChoosedElement.type)
    {
    case gdr_index_types::light:
      ShowEditResourceLight();
      break;
    case gdr_index_types::object_transform:
      ShowEditResourceObjectTransform();
      break;
    case gdr_index_types::node_transform:
      ShowEditResourceNodeTransform();
      break;
    case gdr_index_types::texture:
      ShowEditResourceTexture();
      break;
    case gdr_index_types::cube_texture:
      ShowEditResourceCubeTexture();
      break;
    case gdr_index_types::shadow_map:
      ShowEditResourceShadowMap();
      break;
    case gdr_index_types::bone_mapping:
      ShowEditResourceBoneMapping();
      break;
    case gdr_index_types::material:
      ShowEditResourceMaterial();
      break;
    case gdr_index_types::model:
      ShowEditResourceModel();
      break;
    case gdr_index_types::animation:
    case gdr_index_types::physic_body:
    case gdr_index_types::draw_command:
    case gdr_index_types::geometry:
    case gdr_index_types::none:
    default:
      break;
    }
  }

  void ShowEditWindow(void)
  {
    if (!IsEditWindow)
      return;

    ImGui::Begin("Editor", &IsEditWindow);

    switch (TypeOfEditor)
    {
    case unit_editor::camera:
      ShowEditCamera();
      break;
    case unit_editor::unit:
      break;
    case unit_editor::resource_index:
      ShowEditResource();
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
    if (!ShowEditor)
    {
      Engine->ResizeImgui(-1, -1);
      if (Engine->KeysClick[VK_ESCAPE])
      {
          ShowEditor = true;
      }
      return;
    }

    Engine->ResizeImgui(max(GameWindowSize.x, 128), max(GameWindowSize.y, 128));
   
    // Disable all light objects
    Engine->ObjectTransformsSystem->GetEditable(
      Engine->ModelsManager->Get(PointLightObject).Render.RootTransform).Transform = mth::matr4f::Scale(0);
    Engine->ObjectTransformsSystem->GetEditable(
      Engine->ModelsManager->Get(DirLightObject).Render.RootTransform).Transform = mth::matr4f::Scale(0);
    Engine->ObjectTransformsSystem->GetEditable(
      Engine->ModelsManager->Get(SpotLightObject).Render.RootTransform).Transform = mth::matr4f::Scale(0);
    Engine->ObjectTransformsSystem->GetEditable(
        Engine->ModelsManager->Get(AxisObject).Render.RootTransform).Transform = mth::matr4f::Scale(0);

    // Visualize choosed light object
    if (TypeOfEditor == resource_index &&
      ChoosedElement.type == gdr_index_types::light &&
      Engine->LightsSystem->IsExist(ChoosedElement) && 
      Engine->LightsSystem->Get(ChoosedElement).ObjectTransformIndex != NONE_INDEX)
    {
      // choose right mesh
      gdr_index ObjectType;
      if (Engine->LightsSystem->Get(ChoosedElement).LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
        ObjectType = DirLightObject;
      if (Engine->LightsSystem->Get(ChoosedElement).LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
        ObjectType = SpotLightObject;
      if (Engine->LightsSystem->Get(ChoosedElement).LightSourceType == LIGHT_SOURCE_TYPE_POINT)
        ObjectType = PointLightObject;

      Engine->ObjectTransformsSystem->GetEditable(
        Engine->ModelsManager->Get(ObjectType).Render.RootTransform).Transform =
        Engine->ObjectTransformsSystem->Get(Engine->LightsSystem->Get(ChoosedElement).ObjectTransformIndex).Transform;
    }

    // Add animations to models

    for (int i = 0; i < Models.size(); i++)
        if (Engine->ModelsManager->IsExist(Models[i].first) && Engine->AnimationManager->IsExist(Models[i].second))
        {
            Engine->AnimationManager->SetAnimationTime(Models[i].first, Models[i].second, Engine->GetTime() * 1000.0);
        }


    if (TypeOfEditor == resource_index &&
        ChoosedElement.type == gdr_index_types::model &&
        Engine->ModelsManager->IsExist(ChoosedElement) &&
        Engine->ModelsManager->Get(ChoosedElement).Render.RootTransform != NONE_INDEX)
    {

        Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(AxisObject).Render.RootTransform).Transform =
            Engine->ObjectTransformsSystem->Get(Engine->ModelsManager->Get(ChoosedElement).Render.RootTransform).Transform;
    }

    if (AskedModel != "")
    {
      LoadModel(AskedModel);
      AskedModel = "";
    }

    if (AskedSaveScene != "")
    {
      SaveScene(AskedSaveScene);
      AskedSaveScene = "";
    }

    if (AskedLoadScene != "")
    {
      LoadScene(AskedLoadScene);
      AskedLoadScene = "";
    }

    if (ClearScene)
    {
      ClearScene = false;
      Clear();
    }

    Engine->AddLambdaForIMGUI([&]()
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        
        modelFileDialog.Display();

        if (modelFileDialog.HasSelected())
        {
            AskedModel = modelFileDialog.GetSelected().string();
            modelFileDialog.ClearSelected();
        }

        saveSceneFileDialog.Display();

        if (saveSceneFileDialog.HasSelected())
        {
          AskedSaveScene = saveSceneFileDialog.GetSelected().string();
          saveSceneFileDialog.ClearSelected();
        }

        loadSceneFileDialog.Display();

        if (loadSceneFileDialog.HasSelected())
        {
          AskedLoadScene = loadSceneFileDialog.GetSelected().string();
          loadSceneFileDialog.ClearSelected();
        }

        //Show All widows
        ShowGameWindow();
        ShowHierarchyWindow();
        ShowRenderParams();
        ShowRenderStats();
        ShowDemoWindow();
        ShowEditWindow();
        ShowResourcesWindow();
        
        // show main menu
        if (ImGui::BeginMainMenuBar()) 
        {
          if (ImGui::BeginMenu("Main"))
          {
            if (ImGui::Button("Play"))
            {
                ShowEditor = false;
            }
            if (ImGui::Button("Save"))
            {
              saveSceneFileDialog.Open();
            }
            if (ImGui::Button("Load"))
            {
              loadSceneFileDialog.Open();
            }
            if (ImGui::Button("Clear"))
            {
              ClearScene = true;
            }
            if (ImGui::Button("Exit"))
            {
                exit(0);
            }

            ImGui::EndMenu();
          }
          if (ImGui::BeginMenu("View"))
          {
            ImGui::Checkbox("Game", &IsGameWindow);
            ImGui::Checkbox("Hierarchy", &IsHierarchyWindow);
            ImGui::Checkbox("Render params", &IsRenderParams);
            ImGui::Checkbox("Render stats", &IsRenderStats);
            ImGui::Checkbox("Editor", &IsEditWindow);
            ImGui::Checkbox("Resources", &IsResourcesWindow);
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
     Engine->ModelsManager->Remove(PointLightObject);
     Engine->ModelsManager->Remove(SpotLightObject);
     Engine->ModelsManager->Remove(DirLightObject);
     Engine->ModelsManager->Remove(AxisObject);
     Clear();
  }
};