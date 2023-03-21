#include "p_header.h"

/* Default constructor. */
gdr::engine::engine()
{
  render::Init(this);
  ModelsManager = new models_manager(this);
  AnimationManager = new animation_manager(this);
  PhysicsManager = new physics_manager(this);
  RaycastManager = new raycast_manager(this);
  UnitsManager = new units_manager(this);
}

/* Destructor */
gdr::engine::~engine()
{
  delete UnitsManager;
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
  PhysicsManager->UpdateGPUData(nullptr);
  bool IsPhysTick = PhysicsManager->Update(GetDeltaTime());

  //update Models and animations
  ModelsManager->UpdateGPUData(nullptr);
  AnimationManager->UpdateGPUData(nullptr);

  // update Units
  UnitsManager->UpdateGPUData(nullptr);

  UnitsManager->Update(IsPhysTick);

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