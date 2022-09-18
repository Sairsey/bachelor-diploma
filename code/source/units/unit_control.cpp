#include "p_header.h"
#include <time.h>

#include "unit_control.h"

void unit_control::Initialize(void)
{
}

void unit_control::Response(void)
{
  mth::vec3f position = mth::vec3f(cos(Engine->GetTime()) * 2, 3, sin(Engine->GetTime()) * 2);
  position *= 3;
  Engine->PlayerCamera.SetView(
    position,
    mth::vec3f(0, 6, 0),
    mth::vec3f(0, 1, 0));
}