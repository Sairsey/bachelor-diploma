#include "p_header.h"

/* Default constructor. */
gdr::engine::engine()
{
  render::Init(this);
  ModelsManager = new models_manager(this);
  AnimationManager = new animation_manager(this);
  PhysicsManager = new physics_manager(this);
}

/* Add new Unit function.
 * ARGUMENTS:
 *   - pointer on Engine
 *       engine* NewEngine
 * RETURNS: None.
 */
void gdr::engine::AddUnit(unit_base* UnitToAdd)
{
  ToAdd.push_back(UnitToAdd);
}

/* Remove Unit function.
 * ARGUMENTS:
 *   - pointer on Unit
 *       unit_base* UnitToRemove
 * RETURNS: None.
 */
void gdr::engine::RemoveUnit(unit_base* UnitToRemove)
{
  ToRemove.push_back(UnitToRemove);
}

/* Destructor */
gdr::engine::~engine()
{
  for (unit_base *unit : Units)
    delete unit;

  Units.clear();
  delete ModelsManager;
  delete AnimationManager;
  delete PhysicsManager;
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
  timer_support::IncreaseFrameCounter();
  timer_support::Response();

  // Update input
  input_support::Response(hWnd);
  input_support::UpdateWheel(win::MouseWheel);

  // update Physics
  PhysicsManager->Update(GetTime());

  // update Units
  if (ToAdd.size())
  {
    render::GetDevice().WaitAllUploadLists();
    render::GetDevice().WaitGPUIdle();
    render::GetDevice().ResizeUpdateBuffer(false);
    PROFILE_CPU_BEGIN("Add new units");
    for (int i = 0; i < ToAdd.size(); i++)
    {
      ToAdd[i]->SetEngine(this);
      ToAdd[i]->Initialize();
      Units.push_back(ToAdd[i]);
    }
    ToAdd.clear();
    PROFILE_CPU_END();
    render::GetDevice().WaitAllUploadLists();
    render::GetDevice().WaitGPUIdle();
    render::GetDevice().ResizeUpdateBuffer(true);
  }
  
  if (ToRemove.size())
  {
    PROFILE_CPU_BEGIN("Remove old units");
    for (int i = 0; i < ToRemove.size(); i++)
    {
      unit_base *unitToFind = ToRemove[i];
      int unitIndex = NONE_INDEX;
      
      // find unit index
      for (int j = 0; j < Units.size() && unitIndex == NONE_INDEX; j++)
        if (unitToFind == Units[j])
          unitIndex = j;

      // if found successfully
      if (unitIndex != NONE_INDEX)
      {
        delete unitToFind;
        Units.erase(Units.begin() + unitIndex);
      }
    }
    ToRemove.clear();
    PROFILE_CPU_END();
  }

  PROFILE_CPU_BEGIN("Units update");
  for (auto& unit : Units)
  {
    PROFILE_CPU_BEGIN(unit->GetName().c_str());
    unit->Response();
    PROFILE_CPU_END();
  }
  PROFILE_CPU_END();

  // Draw Frame
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