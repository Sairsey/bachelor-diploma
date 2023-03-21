#pragma once
#include "../unit_base.h"

enum struct editor_type
{
  none = 0,
  camera,
  resource,
  unit
};

#include "unit_editor_window_game.h"
#include "unit_editor_window_hier.h"
#include "unit_editor_window_render_params.h"
#include "unit_editor_window_render_stats.h"
#include "unit_editor_window_resources.h"
#include "unit_editor_window_edit.h"

#include <json.hpp>
#include <fstream>

// TODO 
// 2) Restore gizmo
// 3) Finish unit editor
// 4) All units should use new system of model storage
// 5) Example scene should have some imgui??
// 6) Units as resource_pool???

class unit_editor : public gdr::unit_base
{
private:
  bool ShowEditor = true;
  bool ClearScene = false;

  std::map<std::string, unit_base *> sub_windows;
  std::string AskedModel = "";
  std::string AskedLoadScene = "";
  std::string AskedSaveScene = "";
  ImGui::FileBrowser modelFileDialog;
  ImGui::FileBrowser saveSceneFileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
  ImGui::FileBrowser loadSceneFileDialog;
public:

  void Initialize(void)
  {
    sub_windows["Game"] = new unit_editor_window_game();
    sub_windows["Hierarchy"] = new unit_editor_window_hier();
    sub_windows["Render params"] = new unit_editor_window_render_params();
    sub_windows["Render stats"] = new unit_editor_window_render_stats();
    sub_windows["Resources"] = new unit_editor_window_resources();
    sub_windows["Edit"] = new unit_editor_window_edit();
    IndicesVars["ChoosedElement"] = NONE_INDEX;
    FloatVars["EditorType"] = (int)editor_type::none;

    FloatVars["OpenModelChoose"] = (int)FALSE;
    FloatVars["AddLight"] = (int)FALSE;
    FloatVars["Remove"] = (int)FALSE;

    modelFileDialog.SetTitle("Choose a model...");
    modelFileDialog.SetTypeFilters({ ".obj", ".fbx", ".glb" });

    saveSceneFileDialog.SetTitle("Save scene to...");
    saveSceneFileDialog.SetTypeFilters({ ".json" });

    loadSceneFileDialog.SetTitle("Load scene from...");
    loadSceneFileDialog.SetTypeFilters({ ".json" });

    for (auto &i : sub_windows)
      Engine->AddUnit(i.second, this);
  }

  void LoadModel(std::string path)
  {
    const std::filesystem::path base = std::filesystem::current_path();
    const std::filesystem::path model_path = std::filesystem::path(path);
    path = std::filesystem::relative(model_path, base).string();

    auto import_data = gdr::ImportModelFromAssimp(path);

    if (!import_data.IsEmpty())
    {
      Engine->GetDevice().WaitAllUploadLists();
      Engine->GetDevice().WaitGPUIdle();
      Engine->GetDevice().ResizeUpdateBuffer(false);
      ID3D12GraphicsCommandList* commandList;
      Engine->GetDevice().BeginUploadCommandList(&commandList);
      IndicesVars["Animation for Resource " + std::to_string(IndicesVars.size() + 1)] = Engine->AnimationManager->Add(import_data);
      PROFILE_BEGIN(commandList, charToWString(path.c_str()).c_str());
      IndicesVars["Resource " + std::to_string(IndicesVars.size())] = Engine->ModelsManager->Add(import_data);
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

      std::string path = sub_data["path"];
      LoadModel(path);
      gdr_index modelKey = IndicesVars["Resource " + std::to_string(IndicesVars.size() - 1)];

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

      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(modelKey).Render.RootTransform).Transform = mth::matr4f::BuildTransform(scale, rot, pos);
    }

    for (int i = 0; i < data["lights"].size(); i++)
    {
      nlohmann::json sub_data = data["lights"][i];
      gdr_index lightIndex = Engine->LightsSystem->Add();
      IndicesVars["Resource " + std::to_string(IndicesVars.size())] = lightIndex;

      Engine->LightsSystem->GetEditable(lightIndex).LightSourceType = sub_data["type"];
      Engine->LightsSystem->GetEditable(lightIndex).ConstantAttenuation = sub_data["attenuation"]["const"];
      Engine->LightsSystem->GetEditable(lightIndex).LinearAttenuation = sub_data["attenuation"]["linear"];
      Engine->LightsSystem->GetEditable(lightIndex).QuadricAttenuation = sub_data["attenuation"]["quad"];

      Engine->LightsSystem->GetEditable(lightIndex).Color = { sub_data["color"][0], sub_data["color"][1], sub_data["color"][2] };

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
      path += ".json";

    nlohmann::json data;
    data["models"] = {};
    data["lights"] = {};

    for (const auto &el : IndicesVars)
    {
      if (el.first == "ChoosedElement")
        continue;
      if (el.second.type == gdr_index_types::model)
      {
        nlohmann::json sub_data;

        sub_data["path"] = Engine->ModelsManager->Get(el.second).Name;

        mth::vec3f pos, scale;
        mth::vec4f rot;

        Engine->ObjectTransformsSystem->Get(Engine->ModelsManager->Get(el.second).Render.RootTransform).Transform.Decompose(pos, rot, scale);

        sub_data["transform"] = { {"pos", {pos.X, pos.Y, pos.Z}}, {"rot", {rot.X, rot.Y, rot.Z, rot.W}}, {"scale", {scale.X, scale.Y, scale.Z}} };

        data["models"].push_back(sub_data);
      }
      if (el.second.type == gdr_index_types::light)
      {
        nlohmann::json sub_data;

        sub_data["type"] = Engine->LightsSystem->Get(el.second).LightSourceType;
        sub_data["attenuation"] = {
          {"const", Engine->LightsSystem->Get(el.second).ConstantAttenuation},
          {"linear", Engine->LightsSystem->Get(el.second).LinearAttenuation},
          {"quad", Engine->LightsSystem->Get(el.second).QuadricAttenuation}
        };

        sub_data["color"] = { Engine->LightsSystem->Get(el.second).Color.R, Engine->LightsSystem->Get(el.second).Color.G, Engine->LightsSystem->Get(el.second).Color.B };

        mth::vec3f pos = { 0, 0, 0 }, scale = { 1, 1, 1 };
        mth::vec4f rot = { 0, 0, 0, 1 };

        if (Engine->LightsSystem->Get(el.second).ObjectTransformIndex != NONE_INDEX)
          Engine->ObjectTransformsSystem->Get(Engine->LightsSystem->Get(el.second).ObjectTransformIndex).Transform.Decompose(pos, rot, scale);

        sub_data["transform"] = { {"pos", {pos.X, pos.Y, pos.Z}}, {"rot", {rot.X, rot.Y, rot.Z, rot.W}}, {"scale", {scale.X, scale.Y, scale.Z}} };

        sub_data["inner_cone"] = Engine->LightsSystem->Get(el.second).AngleInnerCone;
        sub_data["outer_cone"] = Engine->LightsSystem->Get(el.second).AngleOuterCone;

        sub_data["shadow_offset"] = Engine->LightsSystem->Get(el.second).ShadowMapOffset;

        sub_data["is_shadow"] = Engine->LightsSystem->Get(el.second).ShadowMapIndex != NONE_INDEX;
        if (Engine->LightsSystem->Get(el.second).ShadowMapIndex != NONE_INDEX)
          sub_data["shadow_size"] = {
            {"w",  Engine->ShadowMapsSystem->Get(Engine->LightsSystem->Get(el.second).ShadowMapIndex).W},
            {"h", Engine->ShadowMapsSystem->Get(Engine->LightsSystem->Get(el.second).ShadowMapIndex).H} };

        data["lights"].push_back(sub_data);
      }
    }

    std::ofstream o(path);
    o << std::setw(4) << data << std::endl;
  }

  void Clear()
  {
    for (const auto& el : IndicesVars)
    {
      if (el.first == "ChoosedElement")
        continue;

      if (el.second.type == gdr_index_types::model)
        Engine->ModelsManager->Remove(el.second);
      else if (el.second.type == gdr_index_types::light)
        Engine->LightsSystem->Remove(el.second);
      else if (el.second.type == gdr_index_types::animation)
        Engine->AnimationManager->Remove(el.second);
      else
        GDR_FAILED("UNKNOWN ELEMENT TYPE TO DELETE");

      IndicesVars.erase(el.first);
    }
  }

  void Response(void)
  {
    if (!ShowEditor)
    {
      Engine->ResizeImgui(-1, -1);
      if (Engine->KeysClick[VK_ESCAPE])
          ShowEditor = true;
      return;
    }

    Engine->ResizeImgui((int)max(Float2Vars["GameWindowSize"].X, 128.0f), (int)max(Float2Vars["GameWindowSize"].Y, 128.0f));
   
    for (const auto &el : IndicesVars)
      if (el.second.type == gdr_index_types::model)
      {
        if (IndicesVars.find(std::string("Animation for ") + el.first) != IndicesVars.end())
        {
          Engine->AnimationManager->SetAnimationTime(el.second, IndicesVars[std::string("Animation for ") + el.first], Engine->GetTime() * 1000.0f);
        }
      }

    if (FloatVars["OpenModelChoose"])
    {
      modelFileDialog.Open();
      FloatVars["OpenModelChoose"] = 0;
    }

    if (FloatVars["AddLight"])
    {
      IndicesVars["Resource " + std::to_string(IndicesVars.size())] = Engine->LightsSystem->Add();
      FloatVars["AddLight"] = 0;
    }

    if (FloatVars["Remove"])
    {
      std::string EraseName = "";
      for (const auto& element : IndicesVars)
        if (element.first != "ChoosedElement" && 
          element.second.type == IndicesVars["ChoosedElement"].type &&
          element.second.value == IndicesVars["ChoosedElement"].value)
        {
          EraseName = element.first;
          break;
        }

      if (EraseName != "")
      {
        if (IndicesVars[EraseName].type == gdr_index_types::model)
        {
          Engine->ModelsManager->Remove(IndicesVars[EraseName]);
          Engine->AnimationManager->Remove(IndicesVars["Animation for " + EraseName]);
          IndicesVars.erase("Animation for " + EraseName);
        }
        else if (IndicesVars[EraseName].type == gdr_index_types::light)
          Engine->LightsSystem->Remove(IndicesVars[EraseName]);
        else
          GDR_FAILED("UNKNOWN ELEMENT TYPE TO DELETE");
        IndicesVars.erase(EraseName);
      }

      IndicesVars["ChoosedElement"] = NONE_INDEX;
      FloatVars["Remove"] = 0;
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
            for (const auto &i : sub_windows)
              ImGui::Checkbox(i.first.c_str(), reinterpret_cast<bool *>(&i.second->FloatVars["Show"]));
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
    Clear();
  }
};