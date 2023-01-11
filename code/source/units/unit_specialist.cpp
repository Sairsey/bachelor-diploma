#include "p_header.h"
#if 0
#include <Windows.h>
#include "unit_specialist.h"
#include <Mmsystem.h>

#pragma comment(lib, "winmm.lib")

void unit_specialist::Initialize(void)
{
  Crash = Engine->ObjectSystem->CreateObjectFromFile("bin/models/specialist/specialist.glb");
  Crash2 = Engine->ObjectSystem->CreateObjectFromFile("bin/models/specialist/specialist2.glb");
  Crash3 = Engine->ObjectSystem->CreateObjectFromFile("bin/models/specialist/specialist3.glb");
}

void unit_specialist::Response(void)
{
  static bool first = true;
  static double time_start = 0;

  if (first)
  {
    PlaySound(TEXT("bin/sounds/specialist.wav"), NULL, SND_FILENAME | SND_ASYNC);
    time_start = Engine->GetTime();
    first = false;
  }

  Engine->PlayerCamera.SetView({0, 2, 6}, {0, 1.9, 5.1}, {0, 1, 0});

  if (Engine->GetTime() - time_start >= 0)
  {
    Engine->ObjectSystem->SetAnimationTime(Crash, (Engine->GetTime() - time_start) * 1000.0);
    Engine->ObjectSystem->SetAnimationTime(Crash2, (Engine->GetTime() - time_start) * 1000.0);
    Engine->ObjectSystem->SetAnimationTime(Crash3, (Engine->GetTime() - time_start) * 1000.0);
  }

  // float radius = 5;
  Engine->ObjectSystem->NodesPool[Crash3].GetTransformEditable() = mth::matr::Translate({ -2, 0 ,0 });
  Engine->ObjectSystem->NodesPool[Crash2].GetTransformEditable() = mth::matr::Translate({ 2, 0 ,0 });
}
#endif