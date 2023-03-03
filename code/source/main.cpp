#include "p_header.h"
#include "units/stats/unit_log_time.h"
#include "units/stats/unit_frame_times.h"
#include "units/stats/unit_resource_viewier.h"
#include "units/stats/unit_render_params.h"
#include "units/stats/unit_light_editor.h"

#include "units/examples/unit_triangle.h"
#include "units/examples/unit_light_example.h"
#include "units/examples/unit_model_loading.h"
#include "units/examples/unit_frustum_cull.h"
#include "units/examples/unit_occlusion_cull.h"
#include "units/examples/unit_runtime_add_remove.h"
#include "units/examples/unit_pbr_metalness.h"
#include "units/examples/unit_pbr_specular.h"
#include "units/examples/unit_normal_map.h"
#include "units/examples/unit_load_any.h"
#include "units/examples/unit_transparent.h"
#include "units/examples/unit_model_splitted.h"
#include "units/examples/unit_specialist.h"
#include "units/examples/unit_pit.h"
#include "units/examples/unit_choose.h"

#include "units/editor/unit_editor.h"
#include "units/editor/unit_scripted.h"

#include "units/thriller/unit_thriller_stage.h"
#include "units/thriller/unit_thriller_env.h"
#include "units/thriller/unit_thriller_camera.h"
#include "units/thriller/unit_thriller_dancers.h"

#include "units/shooter/unit_shooter_first_person.h"
#include "units/shooter/unit_shooter_enviroment.h"
#include "units/shooter/unit_shooter_enemy_spawner.h"
#include "units/shooter/unit_shooter_gun.h"

#include "units/fishes/unit_fishes_env.h"
#include "units/fishes/unit_fishes_rotating.h"

#include "units/enviroment/unit_yokohama.h"
#include "units/enviroment/unit_alex_apt.h"

#include "units/cameras/unit_flying_camera.h"
#include "units/cameras/unit_first_person_camera.h"

#include "utils/editor/editors_modules_list.h"

// Notes:
// 1) Error checking
// 2) times profiling on CPU too
// 3) Need a link between Object transform and Node Transform
// 4) I need to add "Occluder" mesh parameter, which is a simplified version of model mesh. On Hier Depth draw only him.
// 5) I cannot implement direct transparency, because I do not know position of my transparent mesh.
// 6) Shadow frustrum culling!!!

#if 0
#include "units/unit_stats.h"
#include "units/unit_city.h"
#include "units/unit_pbr_spheres.h"
#include "units/unit_bathroom.h"
#include "units/unit_chief.h"
#include "units/unit_specialist.h"
#include "units/unit_cubemap_capture.h"
#include "units/unit_physic_spheres.h"

#include "units/unit_pit.h"

enum EXAMPLE
{
  NONE,
  SPECIALIST,
  SHOOTER,
  PIT
};

#endif
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  gdr::engine Engine;
  
#if 0
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
#endif

  //Engine.AddUnit(new unit_model_loading());
  //Engine.AddUnit(new unit_frustum_cull());
  //Engine.AddUnit(new unit_light_example());
  //Engine.AddUnit(new unit_occlusion_cull());
  //Engine.AddUnit(new unit_triangle());
  //Engine.AddUnit(new unit_runtime_add_remove());
  //Engine.AddUnit(new unit_pbr_metalness());
  //Engine.AddUnit(new unit_pbr_specular());
  //Engine.AddUnit(new unit_normal_map());
  //Engine.AddUnit(new unit_load_any());
  //Engine.AddUnit(new unit_transparent());
  //Engine.AddUnit(new unit_model_splitted());
  //Engine.AddUnit(new unit_specialist());
  Engine.AddUnit(new unit_pit());
  
  //Engine.AddUnit(new unit_thriller_stage());
  //Engine.AddUnit(new unit_thriller_env());
  //Engine.AddUnit(new unit_thriller_camera());
  //Engine.AddUnit(new unit_thriller_dancers());
  
  //Engine.AddUnit(new unit_shooter_first_person());
  //Engine.AddUnit(new unit_shooter_enviroment());
  //Engine.AddUnit(new unit_shooter_enemy_spawner(10));
  //Engine.AddUnit(new unit_shooter_gun());

  Engine.AddUnit(new unit_scripted("Hello"));

  //Engine.AddUnit(new unit_fishes_env());
  //Engine.AddUnit(new unit_fishes_rotating());
  
  //Engine.AddUnit(new unit_yokohama());
  Engine.AddUnit(new unit_alex_apt());
  
  //Engine.AddUnit(new unit_choose());
  //Engine.AddUnit(new unit_log_time());

  Engine.AddUnit(new unit_flying_camera());
  //Engine.AddUnit(new unit_first_person_camera());

  //Engine.AddUnit(new unit_editor());
  
  Engine.AddUnit(new unit_light_editor());
  Engine.AddUnit(new unit_frame_times());
  Engine.AddUnit(new unit_resource_viewier());
  Engine.AddUnit(new unit_render_params());
  // math smoke test
  mth::vec3f V1(1, 0, 0);
  mth::vec3f V2(0, 1, 0);
  auto res = V1 cross V2;
  auto res2 = V1 * mth::matr4f::Translate(1);
  GDR_ASSERT(res == mth::vec3f(0, 0, 1));

  return (int)Engine.Run();
}