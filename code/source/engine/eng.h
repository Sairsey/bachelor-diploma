#pragma once

#include <Windows.h>

#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Engine class declaration */
  class engine : 
    public win,
    public render,
    public timer_support,
    public input_support
  {
    private:
      /* Vector of all units we will draw */
      std::vector<unit_base*> ToAdd;
      std::vector<unit_base*> ToRemove;
    public:
      std::vector<unit_base*> Units;
      // Default constructor
      engine();

      /* Add new Unit function.
       * ARGUMENTS:
       *   - pointer on Unit
       *       unit_base *UnitToAdd
       * RETURNS: None.
       */
      void AddUnit(unit_base *UnitToAdd);

      /* Remove Unit function.
       * ARGUMENTS:
       *   - pointer on Unit
       *       unit_base* UnitToRemove
       * RETURNS: None.
       */
      void RemoveUnit(unit_base* UnitToRemove);

      /* Destructor */
      ~engine();

      /* Window Message Cracks */

      /* Initialization function.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      virtual VOID Init(VOID) override;

      /* Change window size handle function.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      VOID Resize(VOID) override;

      VOID ResizeImgui(int W, int H);

      /* Erase background handle function.
       * ARGUMENTS:
       *   - device context of client area:
       *       HDC hDC;
       * RETURNS: None.
       */
      VOID Erase(HDC hDC) override;

      /* Paint window content function.
       * ARGUMENTS:
       *   - device context of client area:
       *       HDC hDC;
       * RETURNS: None.
       */
      VOID Paint(HDC hDC) override;

      /* Activate handle function.
       * ARGUMENTS:
       *   - activation window flag:
       *       BOOL IsActive;
       * RETURNS: None.
       */
      VOID Activate(BOOL IsActive) override;

      /* Timer handle function.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      VOID Timer(VOID) override;

      /* Free CPU time handling function.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      VOID Idle(VOID) override;

      models_manager *ModelsManager;
      animation_manager *AnimationManager;
      physics_manager *PhysicsManager;
      double EngineClock;
  };
}