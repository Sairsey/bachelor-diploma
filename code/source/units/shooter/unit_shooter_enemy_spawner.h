#pragma once
#include "../unit_base.h"
#include "unit_shooter_enemy.h"

class unit_shooter_enemy_spawner : public gdr::unit_base
{
private:
  int EnemyAmount;
  std::vector<unit_shooter_enemy *> Enemies;
  float SpawnRadius = 30;
  float InnerSafeRadius = 50;
  const float UpdateTime = 20; // seconds
  float PreviousTime = 0;

public:

  unit_shooter_enemy_spawner(int amount = 100)
  {
    EnemyAmount = amount;
  }

  mth::vec3f GetRandomPos(void)
  {
    float rx = 2.0f * rand() / RAND_MAX - 1;
    float ry = 2.0f * rand() / RAND_MAX - 1;
    rx *= SpawnRadius;
    ry *= SpawnRadius;
    
    rx += InnerSafeRadius * rx / sqrt(rx * rx + ry * ry);
    ry += InnerSafeRadius * ry / sqrt(rx * rx + ry * ry);
    return mth::vec3f(rx, 1, ry);
  }

  void Initialize()
  {
    for (int i = 0; i < EnemyAmount; i++)
      Engine->UnitsManager->Add(new unit_shooter_enemy(GetRandomPos()), Me);
  }

  void Response(void)
  {
    for (int i = 0; i < EnemyAmount - ChildUnits.size(); i++ )
      Engine->UnitsManager->Add(new unit_shooter_enemy(GetRandomPos()), Me);
  }

  std::string GetName(void)
  {
    return "unit_shooter_enemy_spawner";
  }

  ~unit_shooter_enemy_spawner(void)
  {
  }
};