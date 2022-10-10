#include "p_header.h"

// Constructor
gdr::render_targets_support::render_targets_support(render* Rnd)
{
  Render = Rnd;

  D3D12_CPU_DESCRIPTOR_HANDLE tmpHandle;
  Render->GetDevice().AllocateRenderTargetView(tmpHandle, (int)render_targets_enum::target_count - 1);
  for (int i = 1; i < (int)render_targets_enum::target_count; i++)
  {
    RenderTargetViews[i] = tmpHandle;
    tmpHandle.ptr += Render->GetDevice().GetRTVDescSize();
  }

  Render->GetDevice().AllocateStaticDescriptors((int)render_targets_enum::target_count, tmpHandle, ShaderResourceViewsGPU);
  for (int i = 0; i < (int)render_targets_enum::target_count; i++)
  {
    ShaderResourceViewsCPU[i] = tmpHandle;
    tmpHandle.ptr += Render->GetDevice().GetRTVDescSize();
  }

  // create each texture for each render target
  CreateTextures();
}

// Set active render_target
void gdr::render_targets_support::Set(ID3D12GraphicsCommandList* CommandList, render_targets_enum ResultTarget)
{
  if (ResultTarget == CurrentRT)
    return;

  if (CurrentRT != render_targets_enum::target_none)
  {
    // transit state of previous rt
    Render->GetDevice().TransitResourceState(CommandList, Textures[(int)CurrentRT].Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

    // transit state of current rt
    Render->GetDevice().TransitResourceState(CommandList, Textures[(int)ResultTarget].Resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
  }

  CurrentRT = ResultTarget;

  CommandList->OMSetRenderTargets(1, &RenderTargetViews[(int)CurrentRT], TRUE, &DepthStencilView);
}

// Function to store display buffer rtv
void gdr::render_targets_support::SaveDisplayBuffer(D3D12_CPU_DESCRIPTOR_HANDLE* RTView, ID3D12Resource* Resource)
{
  RenderTargetViews[(int)render_targets_enum::target_display] = *RTView;
  Textures[(int)render_targets_enum::target_display].Resource = Resource;

  // create SRV for display
  {
    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Format = Textures[0].Resource->GetDesc().Format;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipLevels = 1;

    Render->GetDevice().GetDXDevice()->CreateShaderResourceView(Textures[0].Resource, &desc, ShaderResourceViewsCPU[0]);
  }

  CurrentRT = render_targets_enum::target_none;
}

// Function to store Depth stencil buffer 
void gdr::render_targets_support::SaveDepthStencilBuffer(D3D12_CPU_DESCRIPTOR_HANDLE* DSView)
{
  DepthStencilView = *DSView;
}

// Resize frame function
void gdr::render_targets_support::Resize(int W, int H)
{
  DeleteTextures();
  CreateTextures();
}

// Destructor 
gdr::render_targets_support::~render_targets_support(void)
{
  DeleteTextures();
}

void gdr::render_targets_support::CreateTextures()
{
  for (int i = 1; i < (int)render_targets_enum::target_count; i++)
  {
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_RESOURCE_DESC desc = 
      CD3DX12_RESOURCE_DESC::Tex2D(
      Formats[i],
      Render->GetEngine()->Width,
      Render->GetEngine()->Height,
      1,
      1,
      1,
      0,
      flags);
    HRESULT hr = Render->GetDevice().CreateGPUResource(desc, D3D12_RESOURCE_STATE_COMMON, NULL, Textures[i]);
  }
  // create rtv for each render target
  for (int i = 1; i < (int)render_targets_enum::target_count; i++)
  {
    D3D12_RENDER_TARGET_VIEW_DESC desc = {};
    desc.Format = Formats[i];
    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;
    desc.Texture2D.PlaneSlice = 0;

    Render->GetDevice().GetDXDevice()->CreateRenderTargetView(Textures[i].Resource, &desc, RenderTargetViews[i]);
  }

  // create SRV for each render target
  for (int i = 1; i < (int)render_targets_enum::target_count; i++)
  {
    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Format = Textures[i].Resource->GetDesc().Format;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipLevels = 1;

    Render->GetDevice().GetDXDevice()->CreateShaderResourceView(Textures[i].Resource, &desc, ShaderResourceViewsCPU[i]);
  }
}

void gdr::render_targets_support::DeleteTextures()
{
  for (int i = 1; i < (int)render_targets_enum::target_count; i++)
  {
    Render->GetDevice().ReleaseGPUResource(Textures[i]);
  }
}