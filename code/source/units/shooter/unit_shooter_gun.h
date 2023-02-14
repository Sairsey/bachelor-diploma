#pragma once
#include "../unit_base.h"

class unit_shooter_gun : public gdr::unit_base
{
private:
  gdr_index PlayerGun;

  // Ammo size
  const int AmmoSize = 20;
  int RemainingAmmo;

  // Models of bullet
  std::vector<gdr_index> BulletModels;
  std::vector<gdr_index> BulletPhysic; 

  // Some additional Data
  float BulletSpeed = 100; // m/s
  float BulletSize = 0.1; // m
  const float ReloadTime = 1; // s
  float BulletOffset = 0.5; // from Gun
  float StartReloadTime = -10;
public:
  void Initialize(void)
  {
    auto gun_model = gdr::ImportModelFromAssimp("bin/models/gun/gun.glb");
    auto BulletImportData = gdr::ImportModelFromAssimp("bin/models/pbr_sphere/pbr_sphere.glb");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PlayerGun = Engine->ModelsManager->Add(gun_model);
    for (int i = 0; i < AmmoSize; i++)
    {
      BulletModels.push_back(Engine->ModelsManager->Add(BulletImportData));
      BulletPhysic.push_back(Engine->PhysicsManager->AddDynamicSphere(BulletSize));
      Engine->PhysicsManager->GetEditable(BulletPhysic[BulletPhysic.size() - 1]).SetParent(BulletModels[BulletModels.size() - 1]);
      Engine->PhysicsManager->GetEditable(BulletPhysic[BulletPhysic.size() - 1]).GetPhysXBody()->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
    }
    Engine->GetDevice().CloseUploadCommandList();

    for (int i = 0; i < AmmoSize; i++)
      Engine->ModelsManager->GetEditable(BulletModels[i]).Name = "Bullet";

    RemainingAmmo = AmmoSize;
  }

  void Response(void)
  {
    if (Engine->GetPause())
      return;
    
    if (RemainingAmmo > 0) // If we have ammo -> Shoot
    {
      if (Engine->KeysClick[VK_LBUTTON])
      {
        RemainingAmmo--;

        gdr_index PhysicSphere = BulletPhysic[RemainingAmmo];

        Engine->PhysicsManager->GetEditable(PhysicSphere).SetPos(Engine->PlayerCamera.GetPos()
          + Engine->PlayerCamera.GetDir() * BulletOffset * 2.0
          - Engine->PlayerCamera.GetUp() * BulletOffset / 4.0
          + Engine->PlayerCamera.GetRight() * BulletOffset / 4.0);
        Engine->PhysicsManager->GetEditable(PhysicSphere).SetVel(Engine->PlayerCamera.GetDir() * BulletSpeed);

        if (RemainingAmmo == 0) // if last ammo -> Start Reloading
        {
          StartReloadTime = Engine->GetTime();
        }
      }
    }
    else if (Engine->GetTime() - StartReloadTime <= ReloadTime) // if reloading -> Play animation
    {
      ;
    }
    else // if animation ended -> Reload
    {
      RemainingAmmo = AmmoSize;
    }

    // Place Gun
    {
      mth::matr4f rotation = Engine->PlayerCamera.MatrView;
      rotation[3][0] = 0;
      rotation[3][1] = 0;
      rotation[3][2] = 0;
      rotation.Transpose();

      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(PlayerGun).Render.RootTransform).Transform =
        mth::matr4f::Scale(0.1) *
        mth::matr4f::RotateZ(-70) *
        rotation *
        mth::matr4f::Rotate(min((Engine->GetTime() - StartReloadTime) / ReloadTime, 1) * 360, Engine->PlayerCamera.GetRight()) *
        mth::matr4f::Translate(Engine->PlayerCamera.GetPos()
          + Engine->PlayerCamera.GetDir() * BulletOffset / 2.0
          - Engine->PlayerCamera.GetUp() * BulletOffset / 4.0
          + Engine->PlayerCamera.GetRight() * BulletOffset / 2.0);
    }

    // Update bullet-s
    for (int i = 0; i < BulletPhysic.size(); i++)
    {
      gdr_index ModelIndex = BulletModels[i];
      gdr_index TransformIndex = Engine->ModelsManager->Get(ModelIndex).Render.RootTransform;
      Engine->ObjectTransformsSystem->GetEditable(TransformIndex).Transform =
        mth::matr4f::Scale(BulletSize) *
        Engine->PhysicsManager->Get(BulletPhysic[i]).GetTransform();
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