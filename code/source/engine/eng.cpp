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
  for (int i = 0; i < Units.size(); i++) // We can add units while initializing, so lets Initialize in for loop
  {
    Units[i]->Initialize();
  }
  render::GetDevice().WaitAllUploadLists();
  render::GetDevice().WaitGPUIdle();
  render::GetDevice().ResizeUpdateBuffer(true);
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
}

/* Activate handle function.
 * ARGUMENTS:
 *   - activation window flag:
 *       BOOL IsActive;
 * RETURNS: None.
 */
VOID gdr::engine::Activate(BOOL IsActive)
{
  SetPause(!IsActive);
}

/* Timer handle function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Timer(VOID)
{
  PROFILE_CPU_BEGIN("ENGINE TICK");
  // update Time
  timer_support::Response();
  input_support::Response(hWnd);
  input_support::UpdateWheel(win::MouseWheel);

  // update Physics
  //physics::Update(GetDeltaTime());

  PROFILE_CPU_BEGIN("Units update");
  for (auto& unit : Units)
  {
    PROFILE_CPU_BEGIN(unit->GetName().c_str());
    unit->Response();
    PROFILE_CPU_END();
  }
  PROFILE_CPU_END();
  timer_support::IncreaseFrameCounter();
  render::DrawFrame();
  PROFILE_CPU_END();
}

/* Free CPU time handling function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Idle(VOID)
{
  Timer();
}