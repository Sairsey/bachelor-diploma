#pragma once
#include "unit_editor.h"
#include "../unit_base.h"

class unit_editor_window_game : public gdr::unit_base
{
private:
  enum drag_state
  {
    none = 0, 
    center,
    x_axis,
    y_axis,
    z_axis
  } my_drag_state = none;
public:
  void Initialize(void)
  {
    FloatVars["Show"] = 1;

    auto import_data_axis = gdr::ImportSplittedModelFromAssimp("bin/models/light_meshes/axis.obj");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "load axis objects");
    IndicesVars["AxisObjectCenter"] = Engine->ModelsManager->Add(import_data_axis[0]);
    IndicesVars["AxisObjectDragX"] = Engine->ModelsManager->Add(import_data_axis[1]);
    IndicesVars["AxisObjectDragY"] = Engine->ModelsManager->Add(import_data_axis[3]);
    IndicesVars["AxisObjectDragZ"] = Engine->ModelsManager->Add(import_data_axis[2]);
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();

    gdr_index UtilityMeshes[] = { IndicesVars["AxisObjectCenter"], IndicesVars["AxisObjectDragX"], IndicesVars["AxisObjectDragY"], IndicesVars["AxisObjectDragZ"] };

    for (int i = 0; i < _countof(UtilityMeshes); i++)
    {
      auto meshes = Engine->ModelsManager->Get(UtilityMeshes[i]).Render.Meshes;
      for (int j = 0; j < meshes.size(); j++)
      {
        Engine->DrawCommandsSystem->GetEditable(Engine->ModelsManager->Get(UtilityMeshes[i]).Render.Hierarchy[meshes[j]].DrawCommand).Indices.ObjectParamsMask |= OBJECT_PARAMETER_TOP_MOST;
      }
    }
  }

  void Response(void) override
  {
    if (FloatVars["Show"] == 0 || ParentUnit->FloatVars["Show"] == 0)
      return;

    // VISUALIZE GIZMO
    if (ParentUnit->FloatVars["EditorType"] == (int)editor_type::resource &&
      ParentUnit->IndicesVars["ChoosedElement"].type == gdr_index_types::model &&
      Engine->ModelsManager->IsExist(ParentUnit->IndicesVars["ChoosedElement"]) &&
      Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform != NONE_INDEX)
    {
      mth::vec3f pos, scale;
      mth::vec4f rot;
      mth::matr4f matr = Engine->ObjectTransformsSystem->Get(Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform).Transform;
      matr.Decompose(pos, rot, scale);

      Engine->ObjectTransformsSystem->Get(Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform).Transform.Decompose(pos, rot, scale);

      mth::matr4f RotationMat = mth::matr4f::BuildTransform(1, rot, 0);

      mth::vec3f xAxis = { 0.8, 0, 0 };
      mth::vec3f yAxis = { 0, 0.8, 0 };
      mth::vec3f zAxis = { 0, 0, 0.8 };

      xAxis = RotationMat * xAxis;
      yAxis = RotationMat * yAxis;
      zAxis = RotationMat * zAxis;

      
      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(IndicesVars["AxisObjectCenter"]).Render.RootTransform).Transform = mth::matr4f::BuildTransform(0.5, rot, pos);
      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(IndicesVars["AxisObjectDragX"]).Render.RootTransform).Transform = mth::matr4f::BuildTransform(0.5, rot, pos + xAxis);
      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(IndicesVars["AxisObjectDragY"]).Render.RootTransform).Transform = mth::matr4f::BuildTransform(0.5, rot, pos + yAxis);
      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(IndicesVars["AxisObjectDragZ"]).Render.RootTransform).Transform = mth::matr4f::BuildTransform(0.5, rot, pos + zAxis);
    }
    else
    {
      Engine->ObjectTransformsSystem->GetEditable(
        Engine->ModelsManager->Get(IndicesVars["AxisObjectCenter"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);
      Engine->ObjectTransformsSystem->GetEditable(
        Engine->ModelsManager->Get(IndicesVars["AxisObjectDragX"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);
      Engine->ObjectTransformsSystem->GetEditable(
        Engine->ModelsManager->Get(IndicesVars["AxisObjectDragY"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);
      Engine->ObjectTransformsSystem->GetEditable(
        Engine->ModelsManager->Get(IndicesVars["AxisObjectDragZ"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);
    }

    bool MouseKeyHold = Engine->Keys[VK_LBUTTON] && Engine->OldKeys[VK_LBUTTON];

    // PICK GIZMO
    if (my_drag_state == none && MouseKeyHold)
    {
      mth::vec3f ScreenDir;
      ScreenDir.X = 2.0f * (Engine->Mx - ParentUnit->Float2Vars["GameWindowPos"].X) / ParentUnit->Float2Vars["GameWindowSize"].X - 1;
      ScreenDir.Y = -2.0f * (Engine->My - ParentUnit->Float2Vars["GameWindowPos"].Y) / ParentUnit->Float2Vars["GameWindowSize"].Y + 1;
      ScreenDir.Z = 1;
      if (fabs(ScreenDir.X) <= 1 && fabs(ScreenDir.Y) <= 1)
      {
        mth::matr4f VP = Engine->PlayerCamera.GetVP();
        mth::matr4f VPInverse = VP.Inversed();
        mth::vec3f WorldOrg = Engine->PlayerCamera.GetPos();
        mth::vec3f WorldDir = VPInverse * ScreenDir - WorldOrg;

        WorldDir.Normalize();
        std::vector<ray_intersect> Outputs;
        if (Engine->RaycastManager->Raycast(WorldOrg, WorldDir, Engine->PlayerCamera.GetFar(), gdr::raycast_manager::MODELS, &Outputs))
          for (int i = 0; i < Outputs.size(); i++)
          {
            if (Outputs[i].Index == IndicesVars["AxisObjectDragX"])
            {
              my_drag_state = x_axis;
              break;
            }
            if (Outputs[i].Index == IndicesVars["AxisObjectDragY"])
            {
              my_drag_state = y_axis;
              break;
            }
            if (Outputs[i].Index == IndicesVars["AxisObjectDragZ"])
            {
              my_drag_state = z_axis;
              break;
            }
          }          
      }
    }

    // DRAG GIZMO
    if ((my_drag_state == x_axis || my_drag_state == y_axis || my_drag_state == z_axis) && MouseKeyHold)
    {
      // 0) hide others
      if (my_drag_state != x_axis)
        Engine->ObjectTransformsSystem->GetEditable(
          Engine->ModelsManager->Get(IndicesVars["AxisObjectDragX"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);
      if (my_drag_state != y_axis)
        Engine->ObjectTransformsSystem->GetEditable(
          Engine->ModelsManager->Get(IndicesVars["AxisObjectDragY"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);
      if (my_drag_state != z_axis)
        Engine->ObjectTransformsSystem->GetEditable(
          Engine->ModelsManager->Get(IndicesVars["AxisObjectDragZ"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);
      if (my_drag_state != center)
        Engine->ObjectTransformsSystem->GetEditable(
          Engine->ModelsManager->Get(IndicesVars["AxisObjectCenter"]).Render.RootTransform).Transform = mth::matr4f::Scale(0);

      // 1) get axis
      mth::vec3f axis;
      
      if (my_drag_state == x_axis)
        axis = { 1, 0, 0 };
      if (my_drag_state == y_axis)
        axis = { 0, 1, 0 };
      if (my_drag_state == z_axis)
        axis = { 0, 0, 1 };

      mth::vec3f pos, scale;
      mth::vec4f rot;

      // 2) get axis in world space
      {
        mth::matr4f matr = Engine->ObjectTransformsSystem->Get(Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform).Transform;
        matr.Decompose(pos, rot, scale);
        mth::matr4f RotationMat = mth::matr4f::BuildTransform(1, rot, 0);
        axis = RotationMat * axis; 
      }
      
      // 3) get axis in screen space
      mth::vec3f screenAxis;
      mth::vec3f screenPos;
      {
        screenPos = pos * Engine->PlayerCamera.GetVP();
        screenAxis = (pos + axis) * Engine->PlayerCamera.GetVP() - screenPos;
        screenAxis.Normalize();
      }

      // 4) calculate Delta
      mth::vec3f delta = 0;
      {
        mth::vec3f mouseMove = {2.0f * Engine->Mdx / ParentUnit->Float2Vars["GameWindowSize"].X, -2.0f * Engine->Mdy / ParentUnit->Float2Vars["GameWindowSize"].Y, 0};

        mth::matr4f VPInversed = Engine->PlayerCamera.GetVP().Inversed();
        
        // delta in screenspace
        delta = screenAxis * (mouseMove dot screenAxis);
        delta = (screenPos + delta) * VPInversed - pos;
      }

      // 5) Apply
      if (delta != 0)
      {
        Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(ParentUnit->IndicesVars["ChoosedElement"]).Render.RootTransform).Transform =
          mth::matr4f::BuildTransform(scale, rot, pos + delta);
      }
    }

    // FREE GIZMO
    if (my_drag_state != none && !MouseKeyHold)
      my_drag_state = none;

    // PICK OBJECT
    if (my_drag_state == none && Engine->KeysClick[VK_LBUTTON])
    {
      mth::vec3f ScreenDir;
      ScreenDir.X = 2.0f * (Engine->Mx - ParentUnit->Float2Vars["GameWindowPos"].X) / ParentUnit->Float2Vars["GameWindowSize"].X - 1;
      ScreenDir.Y = -2.0f * (Engine->My - ParentUnit->Float2Vars["GameWindowPos"].Y) / ParentUnit->Float2Vars["GameWindowSize"].Y + 1;
      ScreenDir.Z = 1;
      if (fabs(ScreenDir.X) <= 1 && fabs(ScreenDir.Y) <= 1)
      {
        mth::matr4f VP = Engine->PlayerCamera.GetVP();
        mth::matr4f VPInverse = VP.Inversed();
        mth::vec3f WorldOrg = Engine->PlayerCamera.GetPos();
        mth::vec3f WorldDir = VPInverse * ScreenDir - WorldOrg;

        WorldDir.Normalize();
        std::vector<ray_intersect> Outputs;

        if (Engine->RaycastManager->Raycast(WorldOrg, WorldDir, Engine->PlayerCamera.GetFar(), gdr::raycast_manager::MODELS, &Outputs))
          for (int i = 0; i < Outputs.size(); i++)
          {
            if (Outputs[i].Index != IndicesVars["AxisObjectDragX"] &&
                Outputs[i].Index != IndicesVars["AxisObjectDragY"] && 
                Outputs[i].Index != IndicesVars["AxisObjectDragZ"] &&
                Outputs[i].Index != IndicesVars["AxisObjectCenter"])
            {
              ParentUnit->IndicesVars["ChoosedElement"] = Outputs[0].Index;
              ParentUnit->FloatVars["EditorType"] = (int)editor_type::resource;
              break;
            }
          }
      }
    }

    // DRAW GAME WINDOW
    Engine->AddLambdaForIMGUI([&]() {
        bool isShow = FloatVars["Show"];
        if (ImGui::Begin("Game", &isShow))
        {
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
        }
        ImGui::End();
        FloatVars["Show"] = isShow;
      });
  }

  std::string GetName(void)
  {
    return "unit_editor_window_game";
  }

  ~unit_editor_window_game()
  {
    Engine->ModelsManager->Remove(IndicesVars["AxisObjectCenter"]);
    Engine->ModelsManager->Remove(IndicesVars["AxisObjectDragX"]);
    Engine->ModelsManager->Remove(IndicesVars["AxisObjectDragY"]);
    Engine->ModelsManager->Remove(IndicesVars["AxisObjectDragZ"]);
  }
};