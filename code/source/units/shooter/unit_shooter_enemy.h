#pragma once
#include "../unit_base.h"

class unit_shooter_enemy : public gdr::unit_base
{
private:
  gdr::gdr_index EnemyCapsule;
  gdr::gdr_index EnemyModel;
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
    EnemyModel = Engine->ObjectSystem->CreateObjectFromFile("bin/models/shrek/shrek.glb");
    EnemyCapsule = Engine->NewDynamicCapsule(gdr::physic_material(), EnemyWidth / 2.0, (EnemyHeight - EnemyWidth) / 2.0, "EnemyCapsule");
    // disable rotation
    Engine->GetPhysObject(EnemyCapsule).ToggleRotation();
    Engine->GetPhysObject(EnemyCapsule).SetPos(EnemyPos + mth::vec3f(0, EnemyHeight / 2.0, 0));
    Engine->GetPhysObject(EnemyCapsule).SetCollideCallback([&](gdr::gdr_physics_object* Me, gdr::gdr_physics_object* Other)
    {
        if (Other && Other->Name == "Bullet")
        {
          IsDeadAnimation = true;
          IsDeadAnimationStart = Engine->GetTime();
        }
    });
  }

  void Reinitialize(mth::vec3f Position)
  {
    EnemyPos = Position;
    Engine->GetPhysObject(EnemyCapsule).SetPos(EnemyPos + mth::vec3f(0, EnemyHeight / 2.0, 0));
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
    mth::matr4f capsuleTransform = Engine->GetPhysObject(EnemyCapsule).GetTransform();
    EnemyPos = mth::vec3f(capsuleTransform[3][0], capsuleTransform[3][1], capsuleTransform[3][2]);
    if (!IsDead && !IsDeadAnimation)
    {
      // Get Player position
      mth::vec3f PlayerPosition = Engine->PlayerCamera.GetPos();
       
      // Calculate Velocity
      mth::vec3f Velocity = (PlayerPosition - EnemyPos).Normalized();
      Velocity.Y = 0;

      // Move Phys Object
      Engine->GetPhysObject(EnemyCapsule).SetVelocity(Velocity * EnemySpeed + mth::vec3f( 0, Engine->GetPhysObject(EnemyCapsule).GetVelocity().Y, 0 ));

      float sina = -Velocity.X;
      float cosa = Velocity.Z;

      // Rotate and Move Model
      Engine->ObjectSystem->NodesPool[EnemyModel].GetTransformEditable() = 
        mth::matr4f::RotateY(atan2f(sina, cosa) * MTH_R2D) *
        mth::matr4f::Scale(0.05) *
        mth::matr4f::Translate(EnemyPos - mth::vec3f( 0, EnemyHeight / 2.0, 0 ));
      SavedTransform = Engine->ObjectSystem->NodesPool[EnemyModel].GetTransformEditable();
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
      Engine->GetPhysObject(EnemyCapsule).SetPos({0, -10, 0});
      Engine->ObjectSystem->NodesPool[EnemyModel].GetTransformEditable() = mth::matr4f::Scale(1 - x) * SavedTransform;
    }
    else
      Engine->ObjectSystem->NodesPool[EnemyModel].GetTransformEditable() = mth::matr4f::Scale(0);
    
  }

  std::string GetName(void)
  {
    return "unit_shooter_enemy";
  }

  ~unit_shooter_enemy(void)
  {
  }
};