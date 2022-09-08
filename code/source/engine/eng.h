#pragma once

#include <Windows.h>

#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Engine class declaration */
  class engine : public win
  {
    private:
      /* Vector of all units we will draw */
      std::vector<unit_base*> Units;
    public:
      
      /* Add new Unit function.
       * ARGUMENTS:
       *   - pointer on Engine
       *       engine* NewEngine
       * RETURNS: None.
       */
      void AddUnit(unit_base *UnitToAdd);

      /* Destructor */
      ~engine();
  };
}