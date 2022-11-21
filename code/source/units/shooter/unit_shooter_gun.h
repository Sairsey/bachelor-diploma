#pragma once
#include "../unit_base.h"

class unit_shooter_gun : public gdr::unit_base
{
private:
  gdr::gdr_index PlayerGun;

  gdr::gdr_index Bullet;
  gdr::gdr_index BulletSphere;
  float BulletSpeed = 100; // m/s
  float BulletSize = 0.1; // m
  const float ReloadTime = 0.2; // s
  const float FlyTime = 0.5; // s
  float LastShotTime = 0;
  float BulletOffset = 0.5; // from Gun
  bool BulletFly = false;
public:
  void Initialize(void)
  {
    PlayerGun = Engine->ObjectSystem->CreateObjectFromFile("bin/models/gun/gun.glb");
    Bullet = Engine->ObjectSystem->CreateObjectFromFile("bin/models/pbr_sphere/pbr_sphere.glb");
    BulletSphere = Engine->NewDynamicSphere(gdr::physic_material(), BulletSize, "Bullet");
    Engine->GetPhysObject(BulletSphere).SetCollideCallback([&](gdr::gdr_physics_object* Me, gdr::gdr_physics_object* Other)
      {
        BulletFly = false;
      });
  }

  void Response(void)
  {
    if (Engine->GetPause())
      return;
    
    if (!BulletFly && Engine->GetTime() - LastShotTime > ReloadTime)
    {
      // on button press-> fire
      if (Engine->KeysClick[VK_LBUTTON])
      {
        LastShotTime = Engine->GetTime();
        Engine->GetPhysObject(BulletSphere).SetPos(Engine->PlayerCamera.GetPos() 
          + Engine->PlayerCamera.GetDir() * BulletOffset * 2.0
          - Engine->PlayerCamera.GetUp() * BulletOffset / 4.0
          + Engine->PlayerCamera.GetRight() * BulletOffset / 4.0);
        Engine->GetPhysObject(BulletSphere).SetVelocity(Engine->PlayerCamera.GetDir() * BulletSpeed);
        BulletFly = true;
      }
    }

    mth::matr4f rotation = Engine->PlayerCamera.MatrView;
    rotation[3][0] = 0;
    rotation[3][1] = 0;
    rotation[3][2] = 0;
    rotation.Transpose();

    Engine->ObjectSystem->NodesPool[PlayerGun].GetTransformEditable() =
      mth::matr4f::Scale(0.1) *
      mth::matr4f::RotateZ(-70) *
      rotation *
      mth::matr4f::Rotate(sin(min((Engine->GetTime() - LastShotTime) / ReloadTime, 1) * MTH_PI) * 20, Engine->PlayerCamera.GetRight()) *
      mth::matr4f::Translate(Engine->PlayerCamera.GetPos()
        + Engine->PlayerCamera.GetDir() * BulletOffset / 2.0
        - Engine->PlayerCamera.GetUp() * BulletOffset / 4.0
        + Engine->PlayerCamera.GetRight() * BulletOffset / 2.0);

    // Update bullet
    if (BulletFly)
    {
      Engine->ObjectSystem->NodesPool[Bullet].GetTransformEditable() = mth::matr4f::Scale(BulletSize) * Engine->GetPhysObject(BulletSphere).GetTransform();
    }
    else
    {
      Engine->ObjectSystem->NodesPool[Bullet].GetTransformEditable() = mth::matr4f::Scale(0);
      Engine->GetPhysObject(BulletSphere).SetPos({0, 1000, 0});
    }
  }

  std::string GetName(void)
  {
    return "unit_shooter_gun";
  }

  ~unit_shooter_gun(void)
  {
  }
};