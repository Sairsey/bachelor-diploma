#include "p_header.h"
#include <time.h>

#include "unit_control.h"

void unit_control::Initialize(void)
{
}

void unit_control::Response(void)
{
  Engine->GlobalsSystem->CPUData.time = 1.0 * clock() / CLOCKS_PER_SEC;
  mth::vec3f position = mth::vec3f(cos(Engine->GlobalsSystem->CPUData.time) * 2, 3, sin(Engine->GlobalsSystem->CPUData.time) * 2);
  position *= 3;
  Engine->PlayerCamera.SetView(
    position,
    mth::vec3f(0),
    mth::vec3f(0, 1, 0));
}