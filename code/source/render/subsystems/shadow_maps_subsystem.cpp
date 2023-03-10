#include "p_header.h"

// Constructor 
gdr::shadow_maps_subsystem::shadow_maps_subsystem(render* Rnd) : resource_pool_subsystem(Rnd)
{
  Render->GetDevice().AllocateStaticDescriptors((UINT)Render->CreationParams.MaxShadowMapsAmount, ShadowMapTableCPU, ShadowMapTableGPU);
}

// Create shadowmap
gdr_index gdr::shadow_maps_subsystem::Add(int W, int H)
{
  GDR_ASSERT(W >= 16);
  GDR_ASSERT(H >= 16);

  gdr_index NewShadowmapIndex = resource_pool_subsystem::Add();

  // if too much textures
  if (AllocatedSize() > Render->CreationParams.MaxTextureAmount)
  {
    resource_pool_subsystem::Remove(NewShadowmapIndex);
    NewShadowmapIndex = NONE_INDEX;
  }

  if (NewShadowmapIndex != NONE_INDEX)
  {
    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    HRESULT hr = Render->GetDevice().CreateGPUResource(
      CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R32_TYPELESS,
        W, H, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
      D3D12_RESOURCE_STATE_COMMON,
      &depthOptimizedClearValue,
      GetEditable(NewShadowmapIndex).TextureResource);
    if (SUCCEEDED(hr))
    {
      GetEditable(NewShadowmapIndex).W = W;
      GetEditable(NewShadowmapIndex).H = H;
      GetEditable(NewShadowmapIndex).IsDSVInited = false;
      GetEditable(NewShadowmapIndex).IsSrvInited = false;
      GetEditable(NewShadowmapIndex).TextureResource.Resource->SetName(L"Shadow map");
    }
    else
    {
      GDR_FAILED("Failed to allocate DX12 memory for texture");
    }   
  }
  return NewShadowmapIndex;
}

// Delete Texture
void gdr::shadow_maps_subsystem::BeforeRemoveJob(gdr_index index)
{
  if (CPUData.size() > index && index >= 0)
  {
    CPUData[index].IsSrvInited = false;
    CPUData[index].IsDSVInited = false;
    CPUData[index].W = 0;
    CPUData[index].H = 0;
    Render->GetDevice().ReleaseGPUResource(CPUData[index].TextureResource);
  }
}

// Update data on GPU in case we need it 
void gdr::shadow_maps_subsystem::BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList)
{
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i) && !Get(i).IsSrvInited)
    {
      // init srv
      D3D12_CPU_DESCRIPTOR_HANDLE TextureDescr = ShadowMapTableCPU;

      TextureDescr.ptr += Render->GetDevice().GetSRVDescSize() * i;

      D3D12_SHADER_RESOURCE_VIEW_DESC texDesc = {};
      texDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      texDesc.Format = DXGI_FORMAT_R32_FLOAT;
      texDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      texDesc.Texture2D.MipLevels = 1;
      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(Get(i).TextureResource.Resource, &texDesc, TextureDescr);
      GetEditable(i).IsSrvInited = true;

      // init dsv
      D3D12_CPU_DESCRIPTOR_HANDLE DepthDescr = Render->RenderTargetsSystem->DepthStencilView;

      DepthDescr.ptr += Render->GetDevice().GetDSVDescSize() * (i + 1);

      D3D12_DEPTH_STENCIL_VIEW_DESC dsvView = {};
      dsvView.Format = DXGI_FORMAT_D32_FLOAT;
      dsvView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsvView.Flags = D3D12_DSV_FLAG_NONE;
      Render->GetDevice().GetDXDevice()->CreateDepthStencilView(Get(i).TextureResource.Resource, &dsvView, DepthDescr);
      GetEditable(i).IsDSVInited = true;
    }
}

// Destructor 
gdr::shadow_maps_subsystem::~shadow_maps_subsystem(void)
{
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i))
      Render->GetDevice().ReleaseGPUResource(GetEditable(i).TextureResource);
}