#include "p_header.h"

HRESULT gdr::d3dinclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
{
  std::vector<char> data;
  bool res = true;

  std::string filename = "bin/shaders/";
  filename += pFileName;

#ifdef _UNICODE
  if (res)
  {
    size_t count = filename.length();
    wchar_t* pBuffer = new wchar_t[count + 1];
    size_t converted = 0;
    res = mbstowcs_s(&converted, pBuffer, count + 1, filename.c_str(), count) == 0 && converted == count + 1;
    if (res)
    {
      includeFiles.insert(pBuffer);

      res = ReadFileContent(pBuffer, data);
    }

    delete[] pBuffer;
  }
#else
  res = Platform::ReadFileContent(filename.c_str(), data);
  if (res)
  {
    includeFiles.insert(filename);
  }
#endif
  if (res)
  {
    LPVOID pData = malloc(data.size());
    memcpy(pData, data.data(), data.size());

    *ppData = pData;
    *pBytes = (UINT)data.size();
  }

  assert(res);

  return res ? S_OK : E_FAIL;
}

HRESULT gdr::d3dinclude::Close(LPCVOID pData)
{
  free(const_cast<void*>(pData));

  return S_OK;
}