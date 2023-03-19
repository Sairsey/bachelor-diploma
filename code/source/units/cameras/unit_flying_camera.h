#pragma once
#include "../unit_base.h"

class unit_flying_camera : public gdr::unit_base
{
private:
  float CameraSpeed;
  float MinCameraSpeed;
  float MaxCameraSpeed;
  float CameraSpeedStep;
  bool SavingTrack;
  float PrevTrackTime;
  std::vector<mth::vec3f> Track;
public:
  void Initialize(void)
  {
    MinCameraSpeed = 10;
    MaxCameraSpeed = 1000;
    CameraSpeedStep = 10;

    CameraSpeed = MinCameraSpeed;
    SavingTrack = false;
    PrevTrackTime = 0;
  }

  void Response(void)
  {
    if (Engine->Keys[VK_RBUTTON])
    {
      if (Engine->Mdx != 0)
      {
        Engine->PlayerCamera.RotateAroundLocY(-Engine->Mdx * 0.1f);
      }
      if (Engine->Mdy != 0)
      {
        Engine->PlayerCamera.RotateAroundLocRight(-Engine->Mdy * 0.1f);
      }
    }

    if (Engine->Keys['W'])
      Engine->PlayerCamera.TranslateCamera(mth::vec3f(0, 0, CameraSpeed * Engine->GetGlobalDeltaTime()));
    if (Engine->Keys['A'])
      Engine->PlayerCamera.TranslateCamera(mth::vec3f(-CameraSpeed * Engine->GetGlobalDeltaTime(), 0, 0));
    if (Engine->Keys['S'])
      Engine->PlayerCamera.TranslateCamera(mth::vec3f(0, 0, -CameraSpeed * Engine->GetGlobalDeltaTime()));
    if (Engine->Keys['D'])
      Engine->PlayerCamera.TranslateCamera(mth::vec3f(CameraSpeed * Engine->GetGlobalDeltaTime(), 0, 0));

    if (Engine->Keys['R'])
      Engine->PlayerCamera.TranslateCamera(mth::vec3f(0, CameraSpeed * Engine->GetGlobalDeltaTime(), 0));
    if (Engine->Keys['F'])
      Engine->PlayerCamera.TranslateCamera(mth::vec3f(0, -CameraSpeed * Engine->GetGlobalDeltaTime(), 0));

    if (Engine->KeysClick[VK_SHIFT])
    {
      CameraSpeed *= CameraSpeedStep;
      if (CameraSpeed > MaxCameraSpeed)
        CameraSpeed = MinCameraSpeed;
    }

    if (SavingTrack && (Engine->GetTime() - PrevTrackTime) > 1.0f)
    {
      Track.push_back(Engine->PlayerCamera.GetPos());
      PrevTrackTime = Engine->GetTime();
    }

    Engine->AddLambdaForIMGUI([&]() {
      ImGui::Begin("Track Recorder", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
      if (!SavingTrack)
      {
        if (ImGui::Button("Start"))
          SavingTrack = true;
      }
      else
      {
        if (ImGui::Button("Stop"))
        {
          SavingTrack = false;
          FILE* dump;
          fopen_s(&dump, "Camera_log.txt", "wt");
          fprintf_s(dump, "%zd\n", Track.size());
          for (int i = 0; i < Track.size(); i++)
            fprintf_s(dump, "%g %g %g\n", Track[i].X, Track[i].Y, Track[i].Z);
          fclose(dump);
          Track.clear();
        }
      }


      ImGui::End();
      });
  }

  std::string GetName(void)
  {
    return "unit_flying_camera";
  }

  ~unit_flying_camera(void)
  {
  }
};