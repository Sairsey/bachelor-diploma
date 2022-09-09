#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//#define NOMINMAX
// Windows Header Files
#include <windows.h>

#include <tchar.h>
#include <assert.h>
#include <comdef.h>

#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dx12.h>

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