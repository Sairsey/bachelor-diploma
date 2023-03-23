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

  void ShowUnitsRecursive(gdr_index Unit, int &i)
  {
    ImGuiTreeNodeFlags flags = 0;
    if (Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"].type == Unit.type &&
      Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"].value == Unit.value)
      flags |= ImGuiTreeNodeFlags_Selected;

    if (ImGui::TreeNodeEx((void*)(intptr_t)i++, flags, Engine->UnitsManager->Get(Unit)->GetName().c_str()))
    {
      if (ImGui::IsItemClicked())
      {
        Engine->UnitsManager->Get(ParentUnit)->FloatVars["EditorType"] = (int)editor_type::unit;
        Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"] = Unit;
      }

      if (!Engine->UnitsManager->Get(Unit)->IndicesVars.empty())
        if (ImGui::TreeNodeEx((void*)(intptr_t)i++, 0, "Resources"))
        {
          for (const auto& el : Engine->UnitsManager->Get(Unit)->IndicesVars)
          {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"].type == el.second.type &&
              Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"].value == el.second.value)
              flags |= ImGuiTreeNodeFlags_Selected;
            ImGui::TreeNodeEx((void*)(intptr_t)i++, flags, el.first.c_str());
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())         
            {
              Engine->UnitsManager->Get(ParentUnit)->FloatVars["EditorType"] = (int)editor_type::resource;
              Engine->UnitsManager->Get(ParentUnit)->IndicesVars["ChoosedElement"] = el.second;
            }
          }
          ImGui::TreePop();
        }

      for (int j = 0; j < Engine->UnitsManager->Get(Unit)->ChildUnits.size(); j++)
        ShowUnitsRecursive(Engine->UnitsManager->Get(Unit)->ChildUnits[j], i);
      ImGui::TreePop();
    }
  }

  void Response(void) override
  {
    if (FloatVars["Show"] == 0 || Engine->UnitsManager->Get(ParentUnit)->FloatVars["Show"] == 0)
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
            Engine->UnitsManager->Get(ParentUnit)->FloatVars["EditorType"] = (int)editor_type::camera;

          if (ImGui::TreeNodeEx("Units", ImGuiTreeNodeFlags_DefaultOpen))
          {
            int i = 0;
            ShowUnitsRecursive(Engine->UnitsManager->GetSceneRoot(), i);
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