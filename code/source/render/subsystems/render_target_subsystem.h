#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  enum struct render_targets_enum
  {
    target_none = -1,
    target_display = 0,        // r8g8b8a8 unorm
    target_frame_hdr,      // r16g16b16a16f
    target_frame_lum,
    target_frame_lum_2,
    target_frame_lum_4,
    target_frame_lum_8,
    target_frame_lum_16,
    target_frame_lum_final,
    target_frame_tonemap,     // for fxaa
    target_frame_final, 
    target_count,
  };

  struct render_target_params
  {
    DXGI_FORMAT Format;      // Render Target Format
    bool IsFullscreen;       // Is this RT must be Fullscreen size with scale
    mth::vec2f scale;
    mth::vec2<int> size;
  };

  // Multiple render targets support class
  class render_targets_subsystem
  {
  private:
    render* Render; // pointer on Render

    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViews[(long)render_targets_enum::target_count];
    D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceViewsCPU[(long)render_targets_enum::target_count];

    render_targets_enum CurrentRT = render_targets_enum::target_none;

    // function to create all textures allocated for render targets
    void CreateTextures();
    // function to delete all textures allocated for render targets
    void DeleteTextures();
  public:

    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView; // first element in pool. after him we can find depth for shadow maps

    // Formats for each render target view
    render_target_params TargetParams[(long)render_targets_enum::target_count] = {
      {DXGI_FORMAT_R8G8B8A8_UNORM, true, mth::vec2f(1.0)},          // target_display
      {DXGI_FORMAT_R11G11B10_FLOAT, true, mth::vec2f(1.0)},         // target_frame_hdr
      {DXGI_FORMAT_R32_FLOAT, true, mth::vec2f(1.0)},               // target_frame_lum
      {DXGI_FORMAT_R32_FLOAT, true, mth::vec2f(0.5)},               // target_frame_lum 1/2
      {DXGI_FORMAT_R32_FLOAT, true, mth::vec2f(0.25)},              // target_frame_lum 1/4
      {DXGI_FORMAT_R32_FLOAT, true, mth::vec2f(0.125)},             // target_frame_lum 1/8
      {DXGI_FORMAT_R32_FLOAT, true, mth::vec2f(0.0625)},            // target_frame_lum 1/16
      {DXGI_FORMAT_R32_FLOAT, true, mth::vec2f(0.03125)},           // target_frame_lum_final 1/32
      {DXGI_FORMAT_R8G8B8A8_UNORM, true, mth::vec2f(1.0)},          // target_frame_tonemap
      {DXGI_FORMAT_R8G8B8A8_UNORM, true, mth::vec2f(1.0)},          // target_frame_final
    };

    // Textures
    GPUResource Textures[(long)render_targets_enum::target_count];
    
    // Texture used for Hier depth
    GPUResource HierDepthTexture;
    D3D12_CPU_DESCRIPTOR_HANDLE HierDepthCPUDescriptorHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE HierDepthGPUDescriptorHandle;


    // GPU Descriptors
    D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceViewsGPU;

    // Constructor
    render_targets_subsystem(render* Rnd);

    // Function to store display buffer rtv
    void SaveDisplayBuffer(D3D12_CPU_DESCRIPTOR_HANDLE *RTView, ID3D12Resource *Resource);

    // Function to get Render target view of Specific frame
    D3D12_CPU_DESCRIPTOR_HANDLE GetHDRRenderTargetView() { return RenderTargetViews[(int)render_targets_enum::target_frame_hdr]; }

    // Function to store Depth stencil buffer 
    void SaveDepthStencilBuffer(D3D12_CPU_DESCRIPTOR_HANDLE* DSView);

    // Resize frame function
    void Resize(int W, int H);

    // Set active render_target
    void Set(ID3D12GraphicsCommandList *CommandList, render_targets_enum ResultTarget);

    // Destructor 
    ~render_targets_subsystem(void);
  };
}