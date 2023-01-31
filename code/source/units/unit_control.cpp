#include "p_header.h"
#include <time.h>
#include "unit_control.h"


void unit_control::Initialize(void)
{
  MinCameraSpeed = 10;
  MaxCameraSpeed = 1000;
  CameraSpeedStep = 10;

  CameraSpeed = MinCameraSpeed;
  SavingTrack = false;
  PrevTrackTime = 0;
  #if 0
  DefaultLight = Engine->LightsSystem->AddDirectionalLightSource();
  Engine->LightsSystem->GetTransform(DefaultLight).transform = mth::matr4f::RotateX(30);
  Engine->LightsSystem->GetLight(DefaultLight).Color = mth::vec3f(1, 1, 1);
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "skybox Init");
  Engine->GlobalsSystem->CPUData.SkyboxCubemapIndex = Engine->CubeTexturesSystem->Load(
    "bin/cubemaps/Forest/cubemap/px.hdr",
    "bin/cubemaps/Forest/cubemap/nx.hdr",
    "bin/cubemaps/Forest/cubemap/py.hdr",
    "bin/cubemaps/Forest/cubemap/ny.hdr",
    "bin/cubemaps/Forest/cubemap/pz.hdr",
    "bin/cubemaps/Forest/cubemap/nz.hdr");

  Engine->GlobalsSystem->CPUData.IrradienceCubemapIndex = Engine->CubeTexturesSystem->Load(
    "bin/cubemaps/bathroom/irradiance/px.hdr",
    "bin/cubemaps/bathroom/irradiance/nx.hdr",
    "bin/cubemaps/bathroom/irradiance/py.hdr",
    "bin/cubemaps/bathroom/irradiance/ny.hdr",
    "bin/cubemaps/bathroom/irradiance/pz.hdr",
    "bin/cubemaps/bathroom/irradiance/nz.hdr");

  Engine->GlobalsSystem->CPUData.PrefilteredCubemapIndex = Engine->CubeTexturesSystem->LoadMips(
    "bin/cubemaps/bathroom/prefiltered",
    5);

  Engine->GlobalsSystem->CPUData.BRDFLUTIndex = Engine->TexturesSystem->Load("bin/cubemaps/bathroom/brdf.hdr");

  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
  #endif
}

void unit_control::Response(void)
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
        FILE *dump;
        fopen_s(&dump, "Camera_log.txt", "wt");
        fprintf_s(dump,"%d\n", Track.size());
        for (int i = 0; i < Track.size(); i++)
          fprintf_s(dump, "%g %g %g\n", Track[i].X, Track[i].Y, Track[i].Z);
        fclose(dump);
        Track.clear();
      }
    }
    
    
    ImGui::End();
    });
}