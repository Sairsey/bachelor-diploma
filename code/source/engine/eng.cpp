#include "p_header.h"

/* Add new Unit function.
 * ARGUMENTS:
 *   - pointer on Engine
 *       engine* NewEngine
 * RETURNS: None.
 */
void gdr::engine::AddUnit(unit_base* UnitToAdd)
{
  UnitToAdd->SetEngine(this);
  Units.push_back(UnitToAdd);
}

/* Destructor */
gdr::engine::~engine()
{
  for (unit_base *unit : Units)
    delete unit;

  Units.clear();
}