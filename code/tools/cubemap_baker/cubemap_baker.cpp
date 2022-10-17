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
  std::string HDRTexture = AskFile();
  std::string OutputDirectory = AskDir();

  cubemap_baker baker(HDRTexture, OutputDirectory);
  baker.BakePreintegratedBRDF();
  baker.BakePrefilteredColor();
  baker.BakeCubemap();
  baker.BakeIrradience();
}