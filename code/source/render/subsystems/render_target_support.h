#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  enum struct render_targets_enum
  {
    target_none = -1,
    target_display = 0,        // r8g8b8a8 unorm
    target_frame,      // r16g16b16a16f

    target_count,
  };

  // Multiple render targets support class
  class render_targets_support
  {
  private:
    render* Render; // pointer on Render

    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViews[(long)render_targets_enum::target_count];
    D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceViewsCPU[(long)render_targets_enum::target_count];

    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView;

    render_targets_enum CurrentRT = render_targets_enum::target_none;

    // function to create all textures allocated for render targets
    void CreateTextures();
    // function to delete all textures allocated for render targets
    void DeleteTextures();
  public:

    // Formats for each render target view
    DXGI_FORMAT Formats[(long)render_targets_enum::target_count] = {
      DXGI_FORMAT_R8G8B8A8_UNORM,
      DXGI_FORMAT_R11G11B10_FLOAT,
    };

    // Textures
    GPUResource Textures[(long)render_targets_enum::target_count];

    // GPU Descriptors
    D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceViewsGPU;

    // Constructor
    render_targets_support(render* Rnd);

    // Function to store display buffer rtv
    void SaveDisplayBuffer(D3D12_CPU_DESCRIPTOR_HANDLE *RTView, ID3D12Resource *Resource);

    // Function to store Depth stencil buffer 
    void SaveDepthStencilBuffer(D3D12_CPU_DESCRIPTOR_HANDLE* DSView);

    // Resize frame function
    void Resize(int W, int H);

    // Set active render_target
    void Set(ID3D12GraphicsCommandList *CommandList, render_targets_enum ResultTarget);

    // Destructor 
    ~render_targets_support(void);
  };
}