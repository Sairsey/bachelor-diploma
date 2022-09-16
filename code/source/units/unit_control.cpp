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
    mth::vec3f(0, 6, 0),
    mth::vec3f(0, 1, 0));

  {
    static double prev_update_time = 0;
    if (Engine->GlobalsSystem->CPUData.time - prev_update_time >= 1.0)
    {
      wchar_t buf[500];
      swprintf_s(buf, L"GDR, Device Frame %g ms, %g FPS, %d objects",
        Engine->DeviceFrameCounter.GetMSec(),
        1000.0 / Engine->DeviceFrameCounter.GetMSec(),
        Engine->ObjectSystem->CPUPool.size());
      SetWindowText(Engine->hWnd, buf);
      prev_update_time = Engine->GlobalsSystem->CPUData.time;
    }
  }
}