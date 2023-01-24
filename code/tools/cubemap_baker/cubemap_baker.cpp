#include <windows.h>
#include <string.h>
#include <string>
#include <ShlObj.h>
#include "cubemap_baker.h"

std::string AskFile(void)
{
  // Ask for HDR file 
  OPENFILENAME ofn = { 0 };
  TCHAR szFile[260] = { 0 };
  // Initialize remaining fields of OPENFILENAME structure
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = GetConsoleWindow();
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "*.HDR\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = "..\\bin\\cubemaps";
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileName(&ofn) == TRUE)
  {
    // use ofn.lpstrFile here
    return std::string(ofn.lpstrFile);
  }
  return "";
}

static int CALLBACK BrowseFolderCallback(
  HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  LPITEMIDLIST pidlNavigate;
  switch (uMsg)
  {
  case BFFM_INITIALIZED:
  {
    pidlNavigate = (LPITEMIDLIST)lpData;
    if (pidlNavigate != NULL)
      SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)FALSE, (LPARAM)pidlNavigate);
  }
  break;
  }
  return 0;
}

std::string AskDir(void)
{
  char path[MAX_PATH];

  GetCurrentDirectoryA(MAX_PATH, path);

  std::string start_dir = path;

  BROWSEINFO bi = { 0 };
  LPITEMIDLIST pidlStart, pidlSelected;
  pidlStart = ILCreateFromPathA(start_dir.c_str());
  bi.hwndOwner = GetConsoleWindow();
  bi.pszDisplayName = 0;
  bi.lpszTitle = ("Browse for folder...");
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
  bi.lpfn = BrowseFolderCallback;
  bi.lParam = (LPARAM)pidlStart;
  pidlSelected = SHBrowseForFolderA(&bi);
  if (pidlSelected != NULL)
  {
    SHGetPathFromIDListA(pidlSelected, path);
    return path;
  }
  return "";
}

int main(void)
{
  if (MessageBox(NULL, "Build HDR image from cubemap?", "Question", MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
    std::string CubemapDirectory = AskDir();
    std::string OutputDirectory = AskDir();

    float_cube_image cubemap(CubemapDirectory);

    float_image hdr(cubemap.data[0].W * 4, cubemap.data[0].H * 3);

    for (int i = 0; i < hdr.H; i++)
    {
      for (int j = 0; j < hdr.W; j++)
      {
        mth::vec3f Dir(0);
        mth::vec2f uv = {1.0f * j / hdr.W, 1.0f * i / hdr.H};
        float theta = -MTH_PI / 2.0f + uv[1] * MTH_PI;
        float phi = uv[0] * MTH_PI * 2.0f;

        Dir.X = -1.0 * cos(theta) * sin(phi);
        Dir.Y = sin(theta);
        Dir.Z = cos(theta) * cos(phi);

        Dir.Normalize();

        mth::vec3f color = cubemap.SampleCube(Dir);

        hdr.PutPixel(color, j, i);

      }
    }
    hdr.Save(OutputDirectory + "/hdri.hdr");
  }

  std::string HDRTexture = AskFile();
  std::string OutputDirectory = AskDir();

  cubemap_baker baker(HDRTexture, OutputDirectory);
  baker.BakeCubemap();
  baker.BakePreintegratedBRDF();
  baker.BakePrefilteredColor();
  baker.BakeIrradience();
}