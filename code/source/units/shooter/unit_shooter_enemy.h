#pragma once
#include "../unit_base.h"

class unit_shooter_enemy : public gdr::unit_base
{
private:
  gdr_index EnemyCapsule;
  gdr_index EnemyModel;
  double EnemyHeight = 4.1; // meters
  double EnemyWidth = 2;
  double EnemySpeed = 2; // m/s
  mth::vec3f EnemyPos;
  bool IsDeadAnimation = false;
  float IsDeadAnimationStart = 0;
  bool IsDead = false;

  mth::matr4f SavedTransform;
public:

  unit_shooter_enemy(mth::vec3f Position)
  {
    EnemyPos = Position;
  }

  void Initialize()
  {
    static auto shrek_model = gdr::ImportModelFromAssimp("bin/models/shrek/shrek.glb");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    EnemyModel = Engine->ModelsManager->Add(shrek_model);
    Engine->GetDevice().CloseUploadCommandList();
    
    EnemyCapsule = Engine->PhysicsManager->AddDynamicCapsule(EnemyWidth / 2.0, (EnemyHeight - EnemyWidth) / 2.0);
    // disable rotation
    Engine->PhysicsManager->GetEditable(EnemyCapsule).SetParent(EnemyModel);
    Engine->PhysicsManager->GetEditable(EnemyCapsule).ToggleRotation();
    Engine->PhysicsManager->GetEditable(EnemyCapsule).SetPos(EnemyPos + mth::vec3f(0, EnemyHeight / 2.0, 0));
    Engine->PhysicsManager->GetEditable(EnemyCapsule).SetCollideCallback([&](gdr_index Me, gdr_index Other)
    {
        if (Engine->PhysicsManager->IsExist(Other) && 
            Engine->ModelsManager->IsExist(Engine->PhysicsManager->Get(Other).GetParent()) && 
            Engine->ModelsManager->Get(Engine->PhysicsManager->Get(Other).GetParent()).Name == "Bullet")
        {
          IsDeadAnimation = true;
          IsDeadAnimationStart = Engine->GetTime();
        }
    });
  }

  void Reinitialize(mth::vec3f Position)
  {
    EnemyPos = Position;
    Engine->PhysicsManager->GetEditable(EnemyCapsule).SetPos(EnemyPos + mth::vec3f(0, EnemyHeight / 2.0, 0));
    IsDead = false;
    IsDeadAnimation = false;
    IsDeadAnimationStart = false;
  }

  bool IsUnitDead(void)
  {
    return IsDead;
  }

  void Response(void)
  {
    // Get current EnemyPosition
    mth::matr4f capsuleTransform = Engine->PhysicsManager->Get(EnemyCapsule).GetTransform();
    EnemyPos = mth::vec3f(capsuleTransform[3][0], capsuleTransform[3][1], capsuleTransform[3][2]);
    if (!IsDead && !IsDeadAnimation)
    {
      // Get Player position
      mth::vec3f PlayerPosition = Engine->PlayerCamera.GetPos();
       
      // Calculate Velocity
      mth::vec3f Velocity = (PlayerPosition - EnemyPos).Normalized();
      Velocity.Y = 0;

      // Move Phys Object
      Engine->PhysicsManager->GetEditable(EnemyCapsule).SetVelocity(Velocity * EnemySpeed + mth::vec3f( 0, Engine->PhysicsManager->Get(EnemyCapsule).GetVelocity().Y, 0 ));

      float sina = -Velocity.X;
      float cosa = Velocity.Z;

      // Rotate and Move Model

      SavedTransform = mth::matr4f::RotateY(atan2f(sina, cosa) * MTH_R2D) *
          mth::matr4f::Scale(0.05) *
          mth::matr4f::Translate(EnemyPos - mth::vec3f(0, EnemyHeight / 2.0, 0));

      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(EnemyModel).Render.RootTransform).Transform = SavedTransform;
    }
    else if (IsDeadAnimation && !IsDead)
    {
      const float AnimLen = 1; // 3 second to shrink
      float x = (Engine->GetTime() - IsDeadAnimationStart) / AnimLen;
      if (x > 1)
      {
        IsDead = true;
        IsDeadAnimation = false;
      }
      Engine->PhysicsManager->GetEditable(EnemyCapsule).SetPos({0, -10, 0});
      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(EnemyModel).Render.RootTransform).Transform = mth::matr4f::Scale(1 - x) * SavedTransform;
    }
    else
        Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(EnemyModel).Render.RootTransform).Transform = mth::matr4f::Scale(0);
    
  }

  std::string GetName(void)
  {
    return "unit_shooter_enemy";
  }

  ~unit_shooter_enemy(void)
  {
  }
};