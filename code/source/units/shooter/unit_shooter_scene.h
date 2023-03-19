#pragma once
#include "../unit_base.h"
#include "unit_shooter_enemy_spawner.h"
#include "unit_shooter_enviroment.h"
#include "unit_shooter_first_person.h"
#include "unit_shooter_gun.h"

/* Unit class declaration */
class unit_shooter_scene : public gdr::unit_base
{
public:
  /* default constructor */
  unit_shooter_scene(void)
  {
  }

  // Return name of this unit
  virtual std::string GetName(void)
  {
    return "unit_shooter_scene";
  }

  /* Initialization function.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void Initialize(void) override
  {
    Engine->AddUnit(new unit_shooter_enemy_spawner(10), this);
    Engine->AddUnit(new unit_shooter_enviroment(), this);
    Engine->AddUnit(new unit_shooter_first_person(), this);
    Engine->AddUnit(new unit_shooter_gun(), this);
  }

  /* Destructor */
  ~unit_shooter_scene(void) override
  {
  }
};