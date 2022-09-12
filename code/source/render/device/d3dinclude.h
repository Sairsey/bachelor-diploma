#pragma once
#include "def.h"

// Project namespace
namespace gdr
{
  // override D3D standart include 
  class d3dinclude : public ID3DInclude
  {
    HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);

    HRESULT Close(LPCVOID pData);
  public:
    std::set<std::tstring> includeFiles;
  };

}