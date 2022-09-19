#include "p_header.h"
#include <time.h>

#include "unit_control.h"

void unit_control::Initialize(void)
{
  MinCameraSpeed = 10;
  MaxCameraSpeed = 1000;
  CameraSpeedStep = 10;

  CameraSpeed = MinCameraSpeed;
}

void unit_control::Response(void)
{
  if (Engine->Keys[VK_RBUTTON])
  {
    if (Engine->Mdx != 0)
    {
        Engine->PlayerCamera.RotateAroundLocY(-Engine->Mdx * 0.01);
    }
    if (Engine->Mdy != 0)
    {
        Engine->PlayerCamera.RotateAroundLocRight(-Engine->Mdy * 0.01);
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
    if (CameraSpeed >= MaxCameraSpeed)
      CameraSpeed = MinCameraSpeed;
  }




}