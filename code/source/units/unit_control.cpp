#include "p_header.h"
#include <time.h>

#include "unit_control.h"

void unit_control::Initialize(void)
{
  MinCameraSpeed = 10;
  MaxCameraSpeed = 1000;
  CameraSpeedStep = 10;

  CameraSpeed = MinCameraSpeed;
  DefaultLight = Engine->LightsSystem->AddDirectionalLightSource();
  Engine->LightsSystem->GetTransform(DefaultLight).transform = mth::matr4f::RotateX(30);
  Engine->LightsSystem->GetLight(DefaultLight).Color = mth::vec3f(1, 1, 1);
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "skybox Init");
  Engine->GlobalsSystem->CPUData.SkyboxCubemapIndex = Engine->CubeTexturesSystem->Load(
    "bin/cubemaps/alexapt/cubemap/px.hdr",
    "bin/cubemaps/alexapt/cubemap/nx.hdr",
    "bin/cubemaps/alexapt/cubemap/py.hdr",
    "bin/cubemaps/alexapt/cubemap/ny.hdr",
    "bin/cubemaps/alexapt/cubemap/pz.hdr",
    "bin/cubemaps/alexapt/cubemap/nz.hdr");

  Engine->GlobalsSystem->CPUData.IrradienceCubemapIndex = Engine->CubeTexturesSystem->Load(
    "bin/cubemaps/alexapt/irradiance/px.hdr",
    "bin/cubemaps/alexapt/irradiance/nx.hdr",
    "bin/cubemaps/alexapt/irradiance/py.hdr",
    "bin/cubemaps/alexapt/irradiance/ny.hdr",
    "bin/cubemaps/alexapt/irradiance/pz.hdr",
    "bin/cubemaps/alexapt/irradiance/nz.hdr");

  Engine->GlobalsSystem->CPUData.PrefilteredCubemapIndex = Engine->CubeTexturesSystem->LoadMips(
    "bin/cubemaps/alexapt/prefiltered",
    5);

  Engine->GlobalsSystem->CPUData.BRDFLUTIndex = Engine->TexturesSystem->Load("bin/cubemaps/alexapt/brdf.hdr");

  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_control::Response(void)
{
  if (Engine->Keys[VK_RBUTTON])
  {
    if (Engine->Mdx != 0)
    {
        Engine->PlayerCamera.RotateAroundLocY(-Engine->Mdx * 0.1);
    }
    if (Engine->Mdy != 0)
    {
        Engine->PlayerCamera.RotateAroundLocRight(-Engine->Mdy * 0.1);
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




}