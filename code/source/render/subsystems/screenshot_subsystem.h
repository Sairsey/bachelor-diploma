#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  // Global data representation class
  class screenshot_support
  {
  private:
    render* Render; // pointer on Render
    std::vector<uint8_t> Textures[(unsigned long long)render_targets_enum::target_count]; // array of Textures on CPU
    std::vector<render_targets_enum> RequestsToMem;
    std::vector<std::pair<render_targets_enum, std::string>> RequestsToFile;
  public:
    // Constructor
    screenshot_support(render* Rnd);

    // Update data
    void Update();

    // Request Readback of specific RenderTarget to memory
    void RequestReadbackToMem(render_targets_enum RenderTarget) { RequestsToMem.push_back(RenderTarget); }

    // Get Requested Texture array
    std::vector<uint8_t> GetRequestedTexture(render_targets_enum RenderTarget) {return Textures[(unsigned long long)RenderTarget]; }

    // Request Readback of specific RenderTarget to file
    void RequestReadbackToFile(render_targets_enum RenderTarget, std::string FileName) { RequestsToFile.push_back(std::make_pair(RenderTarget, FileName)); }
  };
}