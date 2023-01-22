#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//#define NOMINMAX
// Windows Header Files
#include <windows.h>

#include "PhysX/PxPhysicsAPI.h"


#include <tchar.h>
#include <set>
#include <assert.h>
#include <comdef.h>

#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dx12.h>

#include <chrono>
#include <string>
#include <functional>
#include <unordered_map>

#include "D3D12MemAlloc.h"
#include "utils/math/mth.h"

#define USE_PIX
#include "WinPixEventRuntime/pix3.h"
#include "imgui/imgui.h"


#define PROFILE_MARKERS

#ifdef PROFILE_MARKERS
#define PROFILE_BEGIN(command_list, name) PIXBeginEvent(command_list, 0x00ffff00, (name));
#define PROFILE_END(command_list) PIXEndEvent(command_list);
#define PROFILE_CPU_BEGIN(name) PIXBeginEvent(0x0000ff00, (name));
#define PROFILE_CPU_END() PIXEndEvent();
#else
#define PROFILE_BEGIN(command_list, name) 
#define PROFILE_END(command_list)
#define PROFILE_CPU_BEGIN(name)
#define PROFILE_CPU_END()
#endif


#define D3D_RELEASE(a) if ((a) != nullptr) {\
    (a)->Release();\
    (a) = nullptr;\
}

#if defined(_DEBUG) || defined(_RELEASE)
#define D3D_CHECK(a) \
if (SUCCEEDED(hr))\
{\
    hr = (a);\
    if (!SUCCEEDED(hr))\
    {\
        _com_error err(hr);\
        OutputDebugString(err.ErrorMessage());\
        OutputDebugString(_T("\n"));\
    }\
    assert(SUCCEEDED(hr));\
}
#else
#define D3D_CHECK(a) \
if (SUCCEEDED(hr))\
{\
    hr = (a); \
}
#endif // _DEBUG || _RELEASE

#ifdef _UNICODE
namespace std
{
  using tstring = wstring;
}
#else
namespace std
{
  using tstring = string;
}
#endif // !UNICODE

inline std::wstring charToWString(const char* text)
{
  std::string str(text);
  std::wstring wstr(str.begin(), str.end());
  return wstr;
}

using gdr_index = uint32_t;

enum struct gdr_hier_node_type
{
    node,
    mesh
};

inline UINT NearestPowerOf2(UINT val)
{
  DWORD idx = 0;
  _BitScanReverse(&idx, val);
  UINT res = 1 << idx;

  return (val & ~res) == 0 ? res : (res << 1);
};

inline UINT CalculateMipMapsAmount(UINT W, UINT H)
{
  int res = 0;
  for (res = 0; W >= 1 && H >= 1; res++)
  {
    W /= 2;
    H /= 2;
  }
  return res;
}