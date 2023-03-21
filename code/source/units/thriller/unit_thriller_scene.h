#pragma once
#include "../unit_base.h"
#include "unit_thriller_camera.h"
#include "unit_thriller_dancers.h"
#include "unit_thriller_env.h"
#include "unit_thriller_stage.h"

/* Unit class declaration */
class unit_thriller_scene : public gdr::unit_base
{
public:
  /* default constructor */
  unit_thriller_scene(void)
  {
  }

  // Return name of this unit
  virtual std::string GetName(void)
  {
    return "unit_thriller_scene";
  }

  /* Initialization function.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void Initialize(void) override
  {
    Engine->UnitsManager->Add(new unit_thriller_camera(), Me);
    Engine->UnitsManager->Add(new unit_thriller_stage(), Me);
    Engine->UnitsManager->Add(new unit_thriller_dancers(), Me);
    Engine->UnitsManager->Add(new unit_thriller_env(), Me);
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
  ~unit_thriller_scene(void) override
  {
  }
};