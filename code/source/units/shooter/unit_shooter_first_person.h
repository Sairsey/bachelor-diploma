#pragma once
#include "../unit_base.h"

class unit_shooter_first_person : public gdr::unit_base
{
private:
  gdr_index PlayerCapsule;
  gdr_index PlayerLight;
  double PlayerHeight = 1.83; // meters
  double PlayerWidth = 0.25 * PlayerHeight; // Leonardo da Vinchi said so
  double PlayerSpeed = 5; // m/s
  double JumpPower = 5; // m/s
  bool IsJump = false;
public:
  void Initialize(void)
  {

    PlayerCapsule = Engine->PhysicsManager->AddDynamicCapsule(PlayerWidth / 2.0, (PlayerHeight - PlayerWidth) / 2.0);
    // disable rotation
    Engine->PhysicsManager->GetEditable(PlayerCapsule).ToggleRotation();
    Engine->PhysicsManager->GetEditable(PlayerCapsule).SetPos({ 0, 1, 0 });
    Engine->PhysicsManager->GetEditable(PlayerCapsule).SetCollideCallback([&](gdr_index Me, gdr_index Other)
      {
        IsJump = false;
      });
    Engine->PhysicsManager->GetEditable(PlayerCapsule).ChangeDensity(200);
    Engine->PlayerCamera.SetView({0, 1, 0}, {1, 1, 0}, {0, 1, 0});
    PlayerLight = Engine->LightsSystem->Add();
    Engine->LightsSystem->GetEditable(PlayerLight).LightSourceType = LIGHT_SOURCE_TYPE_POINT;
    Engine->LightsSystem->GetEditable(PlayerLight).Color = { 1, 0.5, 0 };
    Engine->LightsSystem->GetEditable(PlayerLight).ObjectTransformIndex = Engine->ObjectTransformsSystem->Add();
  }

  void Response(void)
  {
    // Enable Pause
    if (Engine->KeysClick[VK_ESCAPE])
    {
      Engine->TogglePause();
    }

    // LOCK MOUSE
    if (!Engine->GetPause())
    {
      // Lock mouse position
      SetCursorPos(Engine->Width / 2, Engine->Height / 2);
      
      // hack to update positions
      {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(Engine->hWnd, &pt);
        Engine->Mx = pt.x;
        Engine->My = pt.y;
      }
      CURSORINFO info;
      info.cbSize = sizeof(CURSORINFO);
      if (GetCursorInfo(&info))
        if (info.flags & CURSOR_SHOWING)
          ShowCursor(0);
    }
    else
    {
      CURSORINFO info;
      info.cbSize = sizeof(CURSORINFO);
      if (GetCursorInfo(&info))
        if (!(info.flags & CURSOR_SHOWING))
          ShowCursor(TRUE);
    }

    // Set Camera position according to Capsule position
    mth::matr4f capsuleTransform = Engine->PhysicsManager->Get(PlayerCapsule).GetTransform();

    mth::vec3f capsulePosition = mth::vec3f(capsuleTransform[3][0], capsuleTransform[3][1], capsuleTransform[3][2]);
    mth::vec3f cameraPosition = capsulePosition + mth::vec3f(0, 1, 0) * PlayerHeight / 2.0;

    Engine->PlayerCamera.SetView(cameraPosition, cameraPosition + Engine->PlayerCamera.GetDir(), {0, 1, 0});
    
    Engine->ObjectTransformsSystem->GetEditable(Engine->LightsSystem->Get(PlayerLight).ObjectTransformIndex).Transform = mth::matr4f::Translate(cameraPosition);

    // MOVING
    if (!Engine->GetPause())
    {
      // Update input
      if (Engine->Mdx != 0)
      {
        Engine->PlayerCamera.RotateAroundLocY(-Engine->Mdx * 0.1);
      }
      if (Engine->Mdy != 0)
      {
        Engine->PlayerCamera.RotateAroundLocRight(-Engine->Mdy * 0.1);
      }

      mth::vec3f VelocityInCameraCS;
      if (Engine->Keys['W'])
        VelocityInCameraCS = mth::vec3f(0, 0, PlayerSpeed);
      else if (Engine->Keys['A'])
        VelocityInCameraCS = mth::vec3f(-PlayerSpeed, 0, 0);
      else if (Engine->Keys['S'])
        VelocityInCameraCS = mth::vec3f(0, 0, -PlayerSpeed);
      else if (Engine->Keys['D'])
        VelocityInCameraCS = mth::vec3f(PlayerSpeed, 0, 0);

      mth::vec3f Velocity =
        Engine->PlayerCamera.GetRight() * VelocityInCameraCS.X +
        Engine->PlayerCamera.GetUp() * VelocityInCameraCS.Y +
        Engine->PlayerCamera.GetDir() * VelocityInCameraCS.Z;

      Velocity.Y = 0;

      static float DeltaTime = 0;

      DeltaTime += Engine->GetDeltaTime();

      if (DeltaTime >= gdr::PHYSICS_TICK)
      {
        DeltaTime -= gdr::PHYSICS_TICK;
        mth::vec3f PreviousVelocity = Engine->PhysicsManager->Get(PlayerCapsule).GetVel();
        PreviousVelocity.X = Velocity.X;
        PreviousVelocity.Z = Velocity.Z;
        Engine->PhysicsManager->GetEditable(PlayerCapsule).SetVel(PreviousVelocity);
 
        if (Engine->Keys[VK_SPACE] && !IsJump)
        {
           Engine->PhysicsManager->GetEditable(PlayerCapsule).AddVel({ 0, sqrtf(2 * 9.81 * 1), 0});
          IsJump = true;
        }
      }
    }
  }

  std::string GetName(void)
  {
    return "unit_shooter_first_person";
  }

  ~unit_shooter_first_person(void)
  {
  }
};