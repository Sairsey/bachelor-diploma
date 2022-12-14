#include "p_header.h"
#include "units/unit_control.h"
#include "units/unit_triangle.h"
#include "units/unit_bread.h"
#include "units/unit_frog.h"
#include "units/unit_stats.h"
#include "units/unit_city.h"
#include "units/unit_pbr_spheres.h"
#include "units/unit_bathroom.h"
#include "units/unit_chief.h"
#include "units/unit_specialist.h"
#include "units/unit_cubemap_capture.h"
#include "units/unit_physic_spheres.h"

#include "units/unit_pit.h"

#include "units/shooter/unit_shooter_first_person.h"
#include "units/shooter/unit_shooter_enviroment.h"
#include "units/shooter/unit_shooter_enemy_spawner.h"
#include "units/shooter/unit_shooter_gun.h"

enum EXAMPLE
{
  NONE,
  SPECIALIST,
  SHOOTER,
  PIT
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  gdr::engine Engine;
  
  EXAMPLE usedExample = NONE;

  if (usedExample == SPECIALIST)
  {
    Engine.AddUnit(new unit_control());
    Engine.AddUnit(new unit_specialist());
    Engine.AddUnit(new unit_bathroom());
  }
  else if (usedExample == SHOOTER)
  {
    Engine.AddUnit(new unit_shooter_first_person());
    Engine.AddUnit(new unit_shooter_enviroment());
    Engine.AddUnit(new unit_shooter_enemy_spawner(1000));
    Engine.AddUnit(new unit_shooter_gun());
  }
  else if (usedExample == PIT)
  {
      Engine.AddUnit(new unit_control());
      Engine.AddUnit(new unit_pit(1000));
      Engine.AddUnit(new unit_phys_spheres());
  }
  else
  {
    Engine.AddUnit(new gdr::unit_base());
    Engine.AddUnit(new unit_control());
    //Engine.AddUnit(new unit_specialist());
    Engine.AddUnit(new unit_bathroom());
    //Engine.AddUnit(new unit_phys_spheres());
    Engine.AddUnit(new unit_cubemap_capture());
    //Engine.AddUnit(new unit_frog()); 
    //Engine.AddUnit(new unit_bread());
    //Engine.AddUnit(new unit_pbr_spheres());
    //Engine.AddUnit(new unit_city());
  }

  // Units


  Engine.AddUnit(new unit_stats());

  // math smoke test
  mth::vec3f V1(1, 0, 0);
  mth::vec3f V2(0, 1, 0);
  auto res = V1 cross V2;
  auto res2 = V1 * mth::matr4f::Translate(1);
  assert(res == mth::vec3f(0, 0, 1));

  return (int)Engine.Run();
}