#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//#define NOMINMAX
// Windows Header Files
#include <windows.h>

#include "PhysX/PxPhysicsAPI.h"

inline void AssertMessage(const char* file, int line, const char* msg)
{
  char buf[300] = {};
  sprintf_s(buf, "Assert Failed!\n File %s\n Line %i\n %s\n", file, line, msg);
  OutputDebugStringA(buf);
  DebugBreak();
  if (MessageBoxA(GetActiveWindow(), buf, "GDR Error Report", MB_OK | MB_ICONERROR))
  {
    abort();
  }
}

#ifdef _DEBUG
#define GDR_ASSERT(expr) \
    if (!(expr)) \
        AssertMessage(__FILE__, __LINE__, #expr)
#define GDR_FAILED(msg) \
        AssertMessage(__FILE__, __LINE__, msg)
#else
#define GDR_ASSERT(expr) ((void)0)
#define GDR_FAILED(msg) ((void)0)
#endif

#include <tchar.h>
#include <set>
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
#include "imgui/imfilebrowser.h"

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
    GDR_ASSERT(SUCCEEDED(hr));\
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

enum struct gdr_hier_node_type
{
    node,
    mesh
};

enum struct gdr_index_types
{
  none = 0,
  model,
  animation,
  physic_body,
  bone_mapping,
  draw_command,
  geometry,
  light,
  material,
  shadow_map,
  texture,
  cube_texture,
  object_transform,
  node_transform,
  unit
};

struct gdr_index
{
    unsigned value = 0;
    gdr_index_types type = gdr_index_types::none;

    gdr_index(unsigned v, gdr_index_types t) : value(v), type(t)
    {
    }

    gdr_index(unsigned v = 0) : value(v), type(gdr_index_types::none)
    {
    }

    gdr_index& operator=(unsigned v)
    {
        value = v;
        return *this;
    }

    gdr_index& operator++(int v)
    {
        value++;
        return *this;
    }

    gdr_index& operator--(int v)
    {
        value--;
        return *this;
    }

    gdr_index& operator++()
    {
        ++value;
        return *this;
    }

    gdr_index& operator--()
    {
        --value;
        return *this;
    }

    gdr_index& operator=(const gdr_index &v)
    {
        value = v.value;
        type = v.type;
        return *this;
    }

    operator unsigned() const noexcept
    {
        return value;
    }

    
    uint64_t pack() const
    {
        return (uint64_t(type) << 32) + value;
    }

    void unpack(uint64_t data)
    {
        type = gdr_index_types(data >> 32);
        data -= uint64_t(type) << 32;
        value = (uint32_t)data;
        return;
    }
};

//using gdr_index = uint32_t;

#ifndef NONE_INDEX
#define NONE_INDEX 0xFFFFFFFF
#endif // !NONE_INDEX

struct ray_intersect
{
  float Distance;
  mth::vec3f Position;
  gdr_index Index;
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