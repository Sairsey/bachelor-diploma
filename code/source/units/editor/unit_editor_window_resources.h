#pragma once
#include "unit_editor.h"
#include "../unit_base.h"

// macro to draw a tree of specific resource
#define ResourceTree(index_type, engine_system, string_name, string_many_name, start_index, is_add, add_lambda) \
  {                                                                                                        \
    ImGuiTreeNodeFlags root_flags = ImGuiTreeNodeFlags_AllowItemOverlap;                                   \
    if (Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"].type == index_type)                                                                 \
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
        if (Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"].type == index_type && Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"].value == i)                                \
          flags |= ImGuiTreeNodeFlags_Selected;                                                            \
        std::string name = string_name " #" + std::to_string(i) + " (";                                    \
        if (engine_system->IsExist(i))                                                                     \
          name += "Alive)";                                                                                \
        else                                                                                               \
          name += "Not alive)";                                                                            \
          ImGui::TreeNodeEx(name.c_str(), flags);                                                          \
          if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())                                       \
          {                                                                                                \
            Engine->UnitsManager->Get(ParentUnit)->FloatVars["EditorType"] = (int)editor_type::resource;                              \
            Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"].type = index_type;                                   \
            Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"].value = i;                                           \
          }                                                                                                \
      }                                                                                                    \
      ImGui::TreePop();                                                                                    \
    }                                                                                                      \
  }

class unit_editor_window_resources : public gdr::unit_base
{
public:
  void Initialize(void)
  {
    FloatVars["Show"] = true;
  }

  void Response(void) override
  {
    if (FloatVars["Show"] == 0 || Engine->UnitsManager->Get(ParentUnit)->FloatVars["Show"] == 0)
      return;

    Engine->AddLambdaForIMGUI([&]() {
      bool isShow = FloatVars["Show"];
      if (ImGui::Begin("Resources", &isShow))
      {
        if (ImGui::TreeNodeEx("GDR", ImGuiTreeNodeFlags_DefaultOpen))
        {
          ResourceTree(gdr_index_types::model, Engine->ModelsManager, "Model", "Models", 0, true, [&]() {Engine->UnitsManager->Get(ParentUnit)->FloatVars["OpenModelChoose"] = true;});
          ResourceTree(gdr_index_types::animation, Engine->AnimationManager, "Animation", "Animations", 0, false, [&]() {});
          ResourceTree(gdr_index_types::physic_body, Engine->PhysicsManager, "Body", "Physics", 1, false, [&]() {});
          ResourceTree(gdr_index_types::bone_mapping, Engine->BoneMappingSystem, "Mapping", "Bone mappings", 1, false, [&]() {});
          ResourceTree(gdr_index_types::draw_command, Engine->DrawCommandsSystem, "Command", "Draw commands", 1, false, [&]() {});
          ResourceTree(gdr_index_types::geometry, Engine->GeometrySystem, "Geometry", "Geometries", 0, false, [&]() {});
          ResourceTree(gdr_index_types::light, Engine->LightsSystem, "Light", "Lights", 1, true, [&]() {Engine->UnitsManager->Get(ParentUnit)->FloatVars["AddLight"] = true; });
          ResourceTree(gdr_index_types::material, Engine->MaterialsSystem, "Material", "Materials", 1, false, [&]() {});
          ResourceTree(gdr_index_types::shadow_map, Engine->ShadowMapsSystem, "Shadow Map", "Shadow Maps", 0, false, [&]() {});
          ResourceTree(gdr_index_types::texture, Engine->TexturesSystem, "Texture", "Textures", 0, false, [&]() {});
          ResourceTree(gdr_index_types::cube_texture, Engine->CubeTexturesSystem, "Cube texture", "Cube Textures", 0, false, [&]() {});
          ResourceTree(gdr_index_types::object_transform, Engine->ObjectTransformsSystem, "Transform", "Object Transforms", 1, false, [&]() {});
          ResourceTree(gdr_index_types::node_transform, Engine->NodeTransformsSystem, "Transform", "Node Transforms", 1, false, [&]() {});

          ImGui::TreePop();
        }

      }
      ImGui::End();
      FloatVars["Show"] = isShow;
      });
  }

  std::string GetName(void)
  {
    return "unit_editor_window_resources";
  }
};