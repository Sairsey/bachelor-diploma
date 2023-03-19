#pragma once
#include "unit_base.h"

namespace gdr
{
  /* Unit class declaration */
  class unit_scene_default : public unit_base
  {
    public:
    /* default constructor */
    unit_scene_default(void)
    {
    }

    // Return name of this unit
    virtual std::string GetName(void)
    {
      return "unit_scene_default";
    }

    /* Initialization function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Initialize(void) override
    {
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
    ~unit_scene_default(void) override
    {
    }
  };
}