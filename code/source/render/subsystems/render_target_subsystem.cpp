#include "p_header.h"

// Constructor
gdr::render_targets_subsystem::render_targets_subsystem(render* Rnd)
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

  Render->GetDevice().AllocateStaticDescriptors(1, HierDepthCPUDescriptorHandle, HierDepthGPUDescriptorHandle);

  // create each texture for each render target
  CreateTextures();
}

// Set active render_target
void gdr::render_targets_subsystem::Set(ID3D12GraphicsCommandList* CommandList, render_targets_enum ResultTarget)
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

  // Viewport
  D3D12_VIEWPORT Viewport;
  // Scissor rect
  D3D12_RECT Rect;

  Viewport.TopLeftX = 0.0f;
  Viewport.TopLeftY = 0.0f;
  Viewport.Height = 
    (FLOAT)(
      TargetParams[(int)CurrentRT].IsFullscreen ?
      Render->GetEngine()->Height * TargetParams[(int)CurrentRT].scale.Y :
      TargetParams[(int)CurrentRT].size.Y);
  Viewport.Width = 
    (FLOAT)(
      TargetParams[(int)CurrentRT].IsFullscreen ?
      Render->GetEngine()->Width * TargetParams[(int)CurrentRT].scale.X :
      TargetParams[(int)CurrentRT].size.X);
  Viewport.MinDepth = 0.0f;
  Viewport.MaxDepth = 1.0f;

  Viewport.Height = max(Viewport.Height, 1);
  Viewport.Width = max(Viewport.Width, 1);

  Rect.left = 0;
  Rect.top = 0;
  Rect.bottom =
    (LONG)(
      TargetParams[(int)CurrentRT].IsFullscreen ?
      Render->GetEngine()->Height * TargetParams[(int)CurrentRT].scale.Y :
      TargetParams[(int)CurrentRT].size.Y);
  Rect.right = 
    (LONG)(
      TargetParams[(int)CurrentRT].IsFullscreen ?
      Render->GetEngine()->Width * TargetParams[(int)CurrentRT].scale.X :
      TargetParams[(int)CurrentRT].size.X);

  Rect.bottom = max(Rect.bottom, 1);
  Rect.right = max(Rect.right, 1);

  CommandList->RSSetViewports(1, &Viewport);
  CommandList->RSSetScissorRects(1, &Rect);

  CommandList->OMSetRenderTargets(1, &RenderTargetViews[(int)CurrentRT], TRUE, &DepthStencilView);
}

// Function to store display buffer rtv
void gdr::render_targets_subsystem::SaveDisplayBuffer(D3D12_CPU_DESCRIPTOR_HANDLE* RTView, ID3D12Resource* Resource)
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
void gdr::render_targets_subsystem::SaveDepthStencilBuffer(D3D12_CPU_DESCRIPTOR_HANDLE* DSView)
{
  DepthStencilView = *DSView;
}

// Resize frame function
void gdr::render_targets_subsystem::Resize(int W, int H)
{
  DeleteTextures();
  CreateTextures();
}

// Destructor 
gdr::render_targets_subsystem::~render_targets_subsystem(void)
{
  DeleteTextures();
}

void gdr::render_targets_subsystem::CreateTextures()
{
  for (int i = 1; i < (int)render_targets_enum::target_count; i++)
  {
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_RESOURCE_DESC desc = 
      CD3DX12_RESOURCE_DESC::Tex2D(
      TargetParams[i].Format,
      TargetParams[i].IsFullscreen ? max(Render->GetEngine()->Width * TargetParams[i].scale.X, 1) : TargetParams[i].size.X,
      TargetParams[i].IsFullscreen ? max(Render->GetEngine()->Height * TargetParams[i].scale.Y, 1) : TargetParams[i].size.Y,
      1,
      1,
      1,
      0,
      flags);
    HRESULT hr = Render->GetDevice().CreateGPUResource(desc, D3D12_RESOURCE_STATE_COMMON, NULL, Textures[i]);
    Textures[i].Resource->SetName(L"RenderTarget");
  }

  // create rtv for each render target
  for (int i = 1; i < (int)render_targets_enum::target_count; i++)
  {
    D3D12_RENDER_TARGET_VIEW_DESC desc = {};
    desc.Format = TargetParams[i].Format;
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

  // create texture for Hier Depth
  {
    D3D12_RESOURCE_DESC textureResource = Render->DepthBuffer.Resource->GetDesc();
    textureResource.Format = DXGI_FORMAT_R32_FLOAT;
    textureResource.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureResource.MipLevels = CalculateMipMapsAmount(Render->DepthBuffer.Resource->GetDesc().Width, Render->DepthBuffer.Resource->GetDesc().Height);;

    Render->GetDevice().CreateGPUResource(
      textureResource,
      D3D12_RESOURCE_STATE_COMMON,
      0,
      HierDepthTexture
    );
    HierDepthTexture.Resource->SetName(L"Hierachical Depth texture");
  }

  // create SRV for Hier Depth texture
  {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = HierDepthTexture.Resource->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = HierDepthTexture.Resource->GetDesc().MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    Render->GetDevice().GetDXDevice()->CreateShaderResourceView(HierDepthTexture.Resource, &srvDesc, HierDepthCPUDescriptorHandle);
  }
}

void gdr::render_targets_subsystem::DeleteTextures()
{
  for (int i = 1; i < (int)render_targets_enum::target_count; i++)
  {
    Render->GetDevice().ReleaseGPUResource(Textures[i]);
  }

  Render->GetDevice().ReleaseGPUResource(HierDepthTexture);
}