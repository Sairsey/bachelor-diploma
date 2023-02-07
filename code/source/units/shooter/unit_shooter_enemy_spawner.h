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
    float rx = 2.0 * rand() / RAND_MAX - 1;
    float ry = 2.0 * rand() / RAND_MAX - 1;
    rx *= SpawnRadius;
    ry *= SpawnRadius;
    
    rx += InnerSafeRadius * rx / sqrt(rx * rx + ry * ry);
    ry += InnerSafeRadius * ry / sqrt(rx * rx + ry * ry);
    return mth::vec3f(rx, 1, ry);
  }

  void Initialize()
  {
    for (int i = 0; i < EnemyAmount; i++)
      Enemies.push_back(new unit_shooter_enemy(GetRandomPos()));

    for (int i = 0; i < Enemies.size(); i++)
      Engine->AddUnit(Enemies[i]);
  }

  void Response(void)
  {
    /*
    if (Engine->GetTime() - PreviousTime > UpdateTime)
    {
      for (int i = 0; i < Enemies.size(); i++)
        if (Enemies[i]->IsUnitDead())
          Enemies[i]->Reinitialize(GetRandomPos());

      PreviousTime = Engine->GetTime();
    }
    */
  }

  std::string GetName(void)
  {
    return "unit_shooter_enemy_spawner";
  }

  ~unit_shooter_enemy_spawner(void)
  {
  }
};