#pragma once

#include "def.h"


/* Project namespace */
namespace gdr
{
  class engine;
  /* Unit class declaration */
  class unit_base
  {
    protected:
      // Current used engine
      engine* Engine;
    public:
      /* default constructor */
      unit_base(void)
      {
      }

      /* Set engine pointer function.
       * ARGUMENTS:
       *   - pointer on Engine
       *       engine* NewEngine
       * RETURNS: None.
       */
      void SetEngine(engine* NewEngine)
      {
        Engine = NewEngine;
      }

      /* Initialization function.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      virtual void Initialize(void)
      {
      }

      /* Response function which will be called on every frame.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      virtual void Response(void)
      {
      }

      /* Destructor */
      virtual ~unit_base(void)
      {
      }
  };
}