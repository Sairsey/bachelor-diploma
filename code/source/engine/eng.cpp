#include "p_header.h"

/* Default constructor. */
gdr::engine::engine()
{
  render::Init(this);
}

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

/* Initialization function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Init(VOID)
{
  for (auto &unit : Units)
  {
    unit->Initialize();
  }
}

/* Change window size handle function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Resize(VOID)
{
  render::Resize(win::Width, win::Height);
}

/* Erase background handle function.
 * ARGUMENTS:
 *   - device context of client area:
 *       HDC hDC;
 * RETURNS: None.
 */
VOID gdr::engine::Erase(HDC hDC)
{

}

/* Paint window content function.
 * ARGUMENTS:
 *   - device context of client area:
 *       HDC hDC;
 * RETURNS: None.
 */
VOID gdr::engine::Paint(HDC hDC)
{
  render::DrawFrame();

  InvalidateRect(win::hWnd, NULL, TRUE);
}

/* Activate handle function.
 * ARGUMENTS:
 *   - activation window flag:
 *       BOOL IsActive;
 * RETURNS: None.
 */
VOID gdr::engine::Activate(BOOL IsActive)
{

}

/* Timer handle function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Timer(VOID)
{

}

/* Free CPU time handling function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Idle(VOID)
{

}