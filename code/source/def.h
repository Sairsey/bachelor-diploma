#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//#define NOMINMAX
// Windows Header Files
#include <windows.h>

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

#include <string>
#include <functional>

#include "D3D12MemAlloc.h"
#include "utils/math/mth.h"


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
