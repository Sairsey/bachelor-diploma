#include "p_header.h"

/* Default constructor. */
gdr::engine::engine()
{
  render::Init(this);
  ModelsManager = new models_manager(this);
  AnimationManager = new animation_manager(this);
  PhysicsManager = new physics_manager(this);
  RaycastManager = new raycast_manager(this);
}

/* Add new Unit function.
 * ARGUMENTS:
 *   - pointer on Engine
 *       engine* NewEngine
 * RETURNS: None.
 */
void gdr::engine::SetScene(unit_base* UnitScene)
{
  if (SceneUnit != nullptr)
    RemoveUnit(SceneUnit);
  SceneUnit = UnitScene;
  ToAdd.push(std::make_pair(SceneUnit, nullptr));
}

/* Add new Unit function.
 * ARGUMENTS:
 *   - pointer on Engine
 *       engine* NewEngine
 * RETURNS: None.
 */
void gdr::engine::AddUnit(unit_base* UnitToAdd, unit_base *ParentUnit)
{
  GDR_ASSERT(SceneUnit != nullptr);
  ToAdd.push(std::make_pair(UnitToAdd, ParentUnit == nullptr ? SceneUnit : ParentUnit));
}

/* Remove Unit function.
 * ARGUMENTS:
 *   - pointer on Unit
 *       unit_base* UnitToRemove
 * RETURNS: None.
 */
void gdr::engine::RemoveUnit(unit_base* UnitToRemove)
{
  GDR_ASSERT(SceneUnit != nullptr);
  for (int i = 0; i < UnitToRemove->ChildUnits.size(); i++)
    RemoveUnit(UnitToRemove->ChildUnits[i]);
  ToRemove.push(UnitToRemove);
}

/* Destructor */
gdr::engine::~engine()
{
  for (unit_base *unit : AllUnits)
    delete unit;

  AllUnits.clear();
  delete ModelsManager;
  delete AnimationManager;
  delete PhysicsManager;
  delete RaycastManager;
}

/* Initialization function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Init(VOID)
{
}

/* Change window size handle function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Resize(VOID)
{
  render::Resize(win::Width, win::Height, false);
}

VOID gdr::engine::ResizeImgui(int W, int H)
{
  render::Resize(W, H, true);
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

void gdr::engine::ResponseUnit(unit_base* Unit)
{
  PROFILE_CPU_BEGIN(Unit->GetName().c_str());
  Unit->Response();
  for (int i = 0; i < Unit->ChildUnits.size(); i++)
    ResponseUnit(Unit->ChildUnits[i]);
  PROFILE_CPU_END();
}

void gdr::engine::ResponsePhysUnit(unit_base* Unit)
{
  PROFILE_CPU_BEGIN(Unit->GetName().c_str());
  Unit->ResponsePhys();
  for (int i = 0; i < Unit->ChildUnits.size(); i++)
    ResponsePhysUnit(Unit->ChildUnits[i]);
  PROFILE_CPU_END();
}

/* Timer handle function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Timer(VOID)
{
  auto cpuStart = std::chrono::system_clock::now();
  PROFILE_CPU_BEGIN("ENGINE TICK");
  // update Time
  timer_support::IncreaseFrameCounter();
  timer_support::Response();

  // Update input
  input_support::Response(hWnd);
  input_support::UpdateWheel(win::MouseWheel);

  // update Physics
  bool IsPhysTick = PhysicsManager->Update(GetDeltaTime());

  //update Models and animations
  ModelsManager->UpdateGPUData(nullptr);
  AnimationManager->UpdateGPUData(nullptr);

  // update Units
  if (!ToAdd.empty())
  {
    render::GetDevice().WaitAllUploadLists();
    render::GetDevice().WaitGPUIdle();
    render::GetDevice().ResizeUpdateBuffer(false);
    PROFILE_CPU_BEGIN("Add new units");
    while (!ToAdd.empty())
    {
      unit_base *UnitToAdd = ToAdd.front().first;
      unit_base *ParentUnit = ToAdd.front().second;
      UnitToAdd->ParentUnit = ParentUnit;
      if (ParentUnit != nullptr)
        ParentUnit->ChildUnits.push_back(UnitToAdd);
      UnitToAdd->Engine = this;
      UnitToAdd->Initialize();
      AllUnits.push_back(UnitToAdd);
      ToAdd.pop();
    }
    PROFILE_CPU_END();
    render::GetDevice().WaitAllUploadLists();
    render::GetDevice().WaitGPUIdle();
    render::GetDevice().ResizeUpdateBuffer(true);
  }
  
  if (!ToRemove.empty())
  {
    PROFILE_CPU_BEGIN("Remove old units");
    while (!ToRemove.empty())
    {
      unit_base* UnitToDelete = ToRemove.front();
      
      GDR_ASSERT(UnitToDelete->ChildUnits.empty());

      // unbind it from parent
      if (UnitToDelete->ParentUnit != nullptr)
      {
        UINT32 index = NONE_INDEX;

        for (UINT32 i = 0; i < UnitToDelete->ParentUnit->ChildUnits.size() && index == NONE_INDEX; i++)
          if (UnitToDelete->ParentUnit->ChildUnits[i] == UnitToDelete)
            index = i;

        if (index != NONE_INDEX)
          UnitToDelete->ParentUnit->ChildUnits.erase(UnitToDelete->ParentUnit->ChildUnits.begin() + index);
      }

      // remove it in list of all units
      {
        UINT32 index = NONE_INDEX;

        for (UINT32 i = 0; i < AllUnits.size() && index == NONE_INDEX; i++)
          if (AllUnits[i] == UnitToDelete)
            index = i;

        if (index != NONE_INDEX)
          AllUnits.erase(AllUnits.begin() + index);
      }

      delete UnitToDelete;
      ToRemove.pop();
    }
    PROFILE_CPU_END();
  }

  if (IsPhysTick)
  {
    PROFILE_CPU_BEGIN("Units phys update tick");
    if (SceneUnit != nullptr)
      ResponsePhysUnit(SceneUnit);
    PROFILE_CPU_END();
  }

  PROFILE_CPU_BEGIN("Units update frame");
  if (SceneUnit != nullptr)
    ResponseUnit(SceneUnit);
  PROFILE_CPU_END();

  // Draw Frame
  render::DrawFrame();
  PROFILE_CPU_END();
  auto cpuEnd = std::chrono::system_clock::now();
  EngineClock = (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(cpuEnd - cpuStart).count());
}

/* Free CPU time handling function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::engine::Idle(VOID)
{
  Timer();
}