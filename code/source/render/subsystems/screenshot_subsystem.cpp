#pragma once
#include "p_header.h"
#include <wincodec.h>
#include "Utils/ScreenGrab12.h"


gdr::screenshot_support::screenshot_support(render* Rnd)
{
  Render = Rnd;
}

void gdr::screenshot_support::Update()
{
  bool Updated[(int)render_targets_enum::target_count] = {false};

  // Update Requests to Memory
  for (int i = 0; i < RequestsToMem.size(); i++)
  {
    if (Updated[(int)RequestsToMem[i]])
      continue;
    else
    {
      DirectX::SaveTextureToMemory(Render->GetDevice().GetPresentQueue(),
        Render->RenderTargets->Textures[(int)RequestsToMem[i]].Resource,
        Textures[(int)RequestsToMem[i]],
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COMMON);

      Updated[(int)RequestsToMem[i]] = true;
    }
  }

  // Update Requests to File
  for (int i = 0; i < RequestsToFile.size(); i++)
  {
    std::wstring widestr = std::wstring(RequestsToFile[i].second.begin(), RequestsToFile[i].second.end());
    if (widestr.substr(widestr.size() - 3) == L"dds")
    {
      DirectX::SaveDDSTextureToFile(Render->GetDevice().GetPresentQueue(),
        Render->RenderTargets->Textures[(int)RequestsToFile[i].first].Resource,
        widestr.c_str(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COMMON);
    }
    else
    {
      GUID container = GUID_ContainerFormatPng;
      DirectX::SaveWICTextureToFile(Render->GetDevice().GetPresentQueue(),
        Render->RenderTargets->Textures[(int)RequestsToFile[i].first].Resource,
        container,
        widestr.c_str(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COMMON);
    }
  }
  RequestsToMem.clear();
  RequestsToFile.clear();
}