#include "p_header.h"
#include "units/unit_control.h"
#include "units/unit_triangle.h"
#include "units/unit_bread.h"
#include "units/unit_frog.h"
#include "units/unit_stats.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  gdr::engine Engine;
  
  // Units
  Engine.AddUnit(new gdr::unit_base());
  Engine.AddUnit(new unit_control());
  Engine.AddUnit(new unit_frog()); 
  Engine.AddUnit(new unit_bread());

  Engine.AddUnit(new unit_stats());

  // math smoke test
  mth::vec3f V1(1, 0, 0);
  mth::vec3f V2(0, 1, 0);
  auto res = V1 cross V2;
  assert(res == mth::vec3f(0, 0, 1));

  return (int)Engine.Run();
}