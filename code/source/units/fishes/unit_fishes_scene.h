#pragma once
#include "../unit_base.h"
#include "unit_fishes_env.h"
#include "unit_fishes_rotating.h"
#include "../enviroment/unit_alex_apt.h"
#include "../cameras/unit_flying_camera.h"

/* Unit class declaration */
class unit_fishes_scene : public gdr::unit_base
{
public:
  /* default constructor */
  unit_fishes_scene(void)
  {
  }

  // Return name of this unit
  virtual std::string GetName(void)
  {
    return "unit_fishes_scene";
  }

  /* Initialization function.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void Initialize(void) override
  {
    Engine->UnitsManager->Add(new unit_alex_apt(), Me);
    Engine->UnitsManager->Add(new unit_flying_camera(), Me);
    Engine->UnitsManager->Add(new unit_fishes_rotating(), Me);
    Engine->UnitsManager->Add(new unit_fishes_env(), Me);
  }

  /* Response function which will be called on every frame.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void Response(void) override
  {
  }

  /* Response function which will be called on every PHYSICS_TICK.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void ResponsePhys(void) override
  {
  }

  /* Destructor */
  ~unit_fishes_scene(void) override
  {
  }
};