#pragma once
#include "../unit_base.h"

class unit_shooter_enemy : public gdr::unit_base
{
private:
  gdr_index EnemyCapsule;
  gdr_index EnemyModel;
  gdr_index EnemyWalkAnimation;
  gdr_index EnemyDieAnimation;
  float EnemyHeight = 4.1f; // meters
  float EnemyWidth = 2;
  float EnemySpeed = 2; // m/s
  mth::vec3f EnemyPos;
  bool IsDeadAnimation = false;
  float IsDeadAnimationStart = 0;
public:

  unit_shooter_enemy(mth::vec3f Position)
  {
    EnemyPos = Position + mth::vec3f(0, EnemyHeight / 2.0f, 0);
  }

  void Initialize()
  {
    static auto enemy_model = gdr::ImportModelFromAssimp("bin/models/zombie/zombie.glb");
    static auto walk_model = gdr::ImportModelFromAssimp("bin/models/zombie/walk.glb");
    static auto die_model = gdr::ImportModelFromAssimp("bin/models/zombie/die.glb");

    EnemyWalkAnimation = Engine->AnimationManager->Add(enemy_model);
    EnemyDieAnimation = Engine->AnimationManager->Add(die_model);

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    EnemyModel = Engine->ModelsManager->Add(enemy_model);
    Engine->GetDevice().CloseUploadCommandList();
    
    EnemyCapsule = Engine->PhysicsManager->AddDynamicCapsule(EnemyWidth / 2.0f, (EnemyHeight - EnemyWidth) / 2.0f);
    // disable rotation
    Engine->PhysicsManager->GetEditable(EnemyCapsule).SetParent(EnemyModel);
    Engine->PhysicsManager->GetEditable(EnemyCapsule).ToggleRotation();
    Engine->PhysicsManager->GetEditable(EnemyCapsule).SetPos(EnemyPos);
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

  void Response(void)
  {
    if (!IsDeadAnimation) // if alive
    {
      mth::vec3f PlayerPosition = Engine->PlayerCamera.GetPos();
      mth::vec3f Dir = (PlayerPosition - EnemyPos).Normalized();
      Dir.Y = 0;
      Dir.Normalize();

      float sina = -Dir.X;
      float cosa = Dir.Z;

      // Visualize our object
      Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(EnemyModel).Render.RootTransform).Transform = 
        mth::matr4f::RotateY(atan2f(sina, cosa) * MTH_R2D) *
        mth::matr4f::Scale(2) *
        mth::matr4f::Translate(Engine->PhysicsManager->Get(EnemyCapsule).GetPos() - mth::vec3f(0, EnemyHeight / 2.0f, 0));

      // Play Animation
      Engine->AnimationManager->SetAnimationTime(EnemyModel, EnemyWalkAnimation, Engine->GetTime() * 1000);
    }
    if (IsDeadAnimation) // if not alive
    {
      float TotalDieTime = Engine->AnimationManager->Get(EnemyDieAnimation).Duration / 1000;

      // Play Animation
      if (Engine->GetTime() - IsDeadAnimationStart <= TotalDieTime)
      {
        Engine->AnimationManager->SetAnimationTime(EnemyModel, EnemyDieAnimation, (Engine->GetTime() - IsDeadAnimationStart) * 1000);
      }
      else // and finally kill
      {
        Engine->UnitsManager->Remove(Me);
      }
    }   
  }

  void ResponsePhys(void)
  {
    if (!IsDeadAnimation)
    {
      mth::vec3f PlayerPosition = Engine->PlayerCamera.GetPos();
      mth::vec3f Velocity = (PlayerPosition - EnemyPos).Normalized();
      Velocity.Y = 0;

      Velocity.Normalize();
      Velocity *= EnemySpeed;

      // move towards player
      mth::vec3f DiffVelocity = Velocity - Engine->PhysicsManager->Get(EnemyCapsule).GetVel();
      mth::vec3f Accel = DiffVelocity / gdr::PHYSICS_TICK;
      Accel *= Engine->PhysicsManager->GetEditable(EnemyCapsule).GetPhysXMaterial()->getStaticFriction() * 0.9f;
      Accel.Y = 0;
      Engine->PhysicsManager->GetEditable(EnemyCapsule).AddForce(
        Accel *
        Engine->PhysicsManager->Get(EnemyCapsule).GetMass());
    }

    EnemyPos = Engine->PhysicsManager->Get(EnemyCapsule).GetPos();
  }

  std::string GetName(void)
  {
    return "unit_shooter_enemy";
  }

  ~unit_shooter_enemy(void)
  {
    Engine->ModelsManager->Remove(EnemyModel);
    Engine->PhysicsManager->Remove(EnemyCapsule);
    Engine->AnimationManager->Remove(EnemyWalkAnimation);
    Engine->AnimationManager->Remove(EnemyDieAnimation);
  }
};