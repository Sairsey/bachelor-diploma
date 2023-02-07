#pragma once
#include "../unit_base.h"

class unit_shooter_gun : public gdr::unit_base
{
private:
  gdr_index PlayerGun;

  gdr_index Bullet;
  gdr_index BulletSphere;
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
    auto gun_model = gdr::ImportModelFromAssimp("bin/models/gun/gun.glb");
    auto bullet_model = gdr::ImportModelFromAssimp("bin/models/pbr_sphere/pbr_sphere.glb");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PlayerGun = Engine->ModelsManager->Add(gun_model);
    Bullet = Engine->ModelsManager->Add(bullet_model);
    Engine->GetDevice().CloseUploadCommandList();

    Engine->ModelsManager->GetEditable(Bullet).Name = "Bullet";

    BulletSphere = Engine->PhysicsManager->AddDynamicSphere(BulletSize);
    Engine->PhysicsManager->GetEditable(BulletSphere).SetParent(Bullet);
    Engine->PhysicsManager->GetEditable(BulletSphere).SetCollideCallback([&](gdr_index Me, gdr_index Other)
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
        Engine->PhysicsManager->GetEditable(BulletSphere).SetPos(Engine->PlayerCamera.GetPos() 
          + Engine->PlayerCamera.GetDir() * BulletOffset * 2.0
          - Engine->PlayerCamera.GetUp() * BulletOffset / 4.0
          + Engine->PlayerCamera.GetRight() * BulletOffset / 4.0);
        Engine->PhysicsManager->GetEditable(BulletSphere).SetVel(Engine->PlayerCamera.GetDir() * BulletSpeed);
        BulletFly = true;
      }
    }

    mth::matr4f rotation = Engine->PlayerCamera.MatrView;
    rotation[3][0] = 0;
    rotation[3][1] = 0;
    rotation[3][2] = 0;
    rotation.Transpose();

    Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(PlayerGun).Render.RootTransform).Transform =
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
      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(Bullet).Render.RootTransform).Transform = 
        mth::matr4f::Scale(BulletSize) *
        Engine->PhysicsManager->Get(BulletSphere).GetTransform();
    }
    else
    {
      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(Bullet).Render.RootTransform).Transform = mth::matr4f::Scale(0);
      Engine->PhysicsManager->GetEditable(BulletSphere).SetPos({0, 1000, 0});
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