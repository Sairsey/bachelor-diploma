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

  if (Engine->KeysClick['I'])
    Engine->Params.IsIndirect = !Engine->Params.IsIndirect;
  
  if (Engine->KeysClick['C'])
    Engine->Params.IsCulling = !Engine->Params.IsCulling;

  static bool isPause = false; 

  if (Engine->KeysClick['P'])
    Engine->TogglePause();

  if (Engine->Params.IsCulling && !Engine->Params.IsIndirect)
    Engine->Params.IsCulling = false;
    
  {
    static double prev_update_time = 0;
    if (Engine->GetTime() - prev_update_time >= 1.0)
    {
      wchar_t buf[500];
      swprintf_s(buf, L"GDR, Device Frame %g ms, %g CPU FPS, %zd objects, Indirect: %s, Culling %s, Paused %s",
        Engine->DeviceFrameCounter.GetMSec(),
        Engine->GetFPS(),
        Engine->ObjectSystem->CPUPool.size(),
        Engine->Params.IsIndirect ? L"Enabled" : L"Disabled",
        Engine->Params.IsCulling ? L"Enabled" : L"Disabled",
        Engine->GetPause() ? L"Enabled" : L"Disabled"
        );
      SetWindowText(Engine->hWnd, buf);
      prev_update_time = Engine->GetTime();
    }
  }
}