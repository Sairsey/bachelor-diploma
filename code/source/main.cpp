#include "p_header.h"

#include "units/unit_scene_default.h"
#include "units/examples/unit_examples_scene.h"
#include "units/fishes/unit_fishes_scene.h"
#include "units/thriller/unit_thriller_scene.h"
#include "units/funk/unit_funk.h"
#include "units/shooter/unit_shooter_scene.h"


#include "units/stats/unit_log_time.h"
#include "units/stats/unit_frame_times.h"
#include "units/stats/unit_resource_viewier.h"
#include "units/stats/unit_render_params.h"
#include "units/stats/unit_light_editor.h"


#include "units/editor/unit_editor.h"
#include "units/editor/unit_blueprint_editor.h"
#include "units/editor/unit_scripted.h"


// Notes:
// 1) Error checking
// 2) times profiling on CPU too
// 3) Need a link between Object transform and Node Transform
// 4) I need to add "Occluder" mesh parameter, which is a simplified version of model mesh. On Hier Depth draw only him.
// 5) I cannot implement direct transparency, because I do not know position of my transparent mesh.
// 6) Shadow frustrum culling!!!

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  gdr::engine Engine;

  Engine.SetScene(new gdr::unit_scene_default());
  //Engine.SetScene(new unit_examples_scene());
  //Engine.SetScene(new unit_thriller_scene());
  //Engine.SetScene(new unit_shooter_scene());
  //Engine.SetScene(new unit_fishes_scene());
  //Engine.SetScene(new unit_funk());

  Engine.AddUnit(new unit_editor());
  Engine.AddUnit(new unit_alex_apt());
  Engine.AddUnit(new unit_flying_camera());

  //Engine.AddUnit(new unit_scripted("Hello"));
  //Engine.AddUnit(new unit_blueprint_editor());

  //Engine.AddUnit(new unit_light_editor());
  //Engine.AddUnit(new unit_frame_times());
  //Engine.AddUnit(new unit_resource_viewier());
  //Engine.AddUnit(new unit_render_params());
  
  return (int)Engine.Run();
}