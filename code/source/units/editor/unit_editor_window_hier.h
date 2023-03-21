#pragma once
#include "unit_editor.h"
#include "../unit_base.h"

class unit_editor_window_hier : public gdr::unit_base
{
public:
  void Initialize(void)
  {
    FloatVars["Show"] = 1;
  }

  void ShowUnitsRecursive(unit_base* Unit, int &i)
  {
    if (ImGui::TreeNodeEx((void*)(intptr_t)i++, 0, Unit->GetName().c_str()))
    {
      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        ParentUnit->FloatVars["EditorType"] = (int)editor_type::unit;

      if (!Unit->IndicesVars.empty())
        if (ImGui::TreeNodeEx((void*)(intptr_t)i++, 0, "Resources"))
        {
          for (const auto& el : Unit->IndicesVars)
          {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (ParentUnit->IndicesVars["ChoosedElement"].type == el.second.type && ParentUnit->IndicesVars["ChoosedElement"].value == el.second.value)
              flags |= ImGuiTreeNodeFlags_Selected;
            ImGui::TreeNodeEx((void*)(intptr_t)i++, flags, el.first.c_str());
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())         
            {
              ParentUnit->FloatVars["EditorType"] = (int)editor_type::resource;
              ParentUnit->IndicesVars["ChoosedElement"] = el.second;
            }
          }
          ImGui::TreePop();
        }

      for (int j = 0; j < Unit->ChildUnits.size(); j++)
        ShowUnitsRecursive(Unit->ChildUnits[j], i);
      ImGui::TreePop();
    }
  }

  void Response(void) override
  {
    if (FloatVars["Show"] == 0 || ParentUnit->FloatVars["Show"] == 0)
      return;

    Engine->AddLambdaForIMGUI([&]() {
      bool isShow = FloatVars["Show"];
      
      if (ImGui::Begin("Hierarchy", &isShow))
      {      
        if (ImGui::TreeNodeEx("GDR", ImGuiTreeNodeFlags_DefaultOpen))
        {
          // add special nodes
          ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);

          if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            ParentUnit->FloatVars["EditorType"] = (int)editor_type::camera;

          if (ImGui::TreeNodeEx("Units", ImGuiTreeNodeFlags_DefaultOpen))
          {
            int i = 0;
            ShowUnitsRecursive(Engine->SceneUnit, i);
            ImGui::TreePop();
          }
          ImGui::TreePop();
        }

      }
      ImGui::End();
      FloatVars["Show"] = isShow;
      });
  }

  std::string GetName(void)
  {
    return "unit_editor_window_hier";
  }
};