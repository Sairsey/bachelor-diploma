#include "p_header.h"

// We pack the UAV counter into the same buffer as the commands rather than create
// a separate 64K resource/heap for it. The counter must be aligned on 4K boundaries,
// so we pad the command buffer (if necessary) such that the counter will be placed
// at a valid location in the buffer.
static UINT AlignForUavCounter(UINT bufferSize)
{
  const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
  return (bufferSize + (alignment - 1)) & ~(alignment - 1);
}

gdr::oit_transparency_subsystem::oit_transparency_subsystem(render* Rnd)
{
  Render = Rnd;
  OITNodesPoolGPUData.Resource = nullptr;
  OITTexture.Resource = nullptr;
  W = 0;
  H = 0;
  Rnd->GetDevice().AllocateStaticDescriptors(1, TextureUAVCPUDescriptor, TextureUAVGPUDescriptor);
  Rnd->GetDevice().AllocateStaticDescriptors(1, NodesPoolUAVCPUDescriptor, NodesPoolUAVGPUDescriptor);
  Rnd->GetDevice().AllocateStaticDescriptors(1, TextureSRVCPUDescriptor, TextureSRVGPUDescriptor);
  Rnd->GetDevice().AllocateStaticDescriptors(1, NodesPoolSRVCPUDescriptor, NodesPoolSRVGPUDescriptor);

  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  heapDesc.NodeMask = 0;
  heapDesc.NumDescriptors = 1;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

  HRESULT hr = S_OK;
  D3D_CHECK(Render->GetDevice().GetDXDevice()->CreateDescriptorHeap(&heapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&D3DDynamicHeap));
}

void gdr::oit_transparency_subsystem::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  if (Render->GlobalsSystem->Get().Width != W || Render->GlobalsSystem->Get().Height != H)
  {
    W = Render->GlobalsSystem->Get().Width;
    H = Render->GlobalsSystem->Get().Height;
    if (OITNodesPoolGPUData.Resource != nullptr)
    {
      Render->GetDevice().ReleaseGPUResource(OITNodesPoolGPUData);
      OITNodesPoolGPUData.Resource = nullptr;
    }
    if (OITTexture.Resource != nullptr)
    {
      Render->GetDevice().ReleaseGPUResource(OITTexture);
      OITTexture.Resource = nullptr;
    }
    // create Texture
    {
      std::vector<uint32_t> InitialData;
      InitialData.resize(W * H);
      for (int i = 0; i < InitialData.size(); i++)
        InitialData[i] = NONE_INDEX;

      HRESULT hr = Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_UINT, W, H, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        OITTexture,
        InitialData.data(),
        InitialData.size());
      OITTexture.Resource->SetName(L"OIT Texture");

      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
      uavDesc.Format = OITTexture.Resource->GetDesc().Format;
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
      uavDesc.Texture2D.MipSlice = 0;
      uavDesc.Texture2D.PlaneSlice = 0;

      Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(
        OITTexture.Resource,
        NULL,
        &uavDesc,
        TextureUAVCPUDescriptor);

      // One more for ClearUnorderedAccessViewUint
      Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(
        OITTexture.Resource,
        NULL,
        &uavDesc,
        D3DDynamicHeap->GetCPUDescriptorHandleForHeapStart());

      D3D12_SHADER_RESOURCE_VIEW_DESC texDesc = {};
      texDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      texDesc.Format = OITTexture.Resource->GetDesc().Format;
      texDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      texDesc.Texture2D.MipLevels = 1;

      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(OITTexture.Resource, &texDesc, TextureSRVCPUDescriptor);
    }
    // create Nodes Pool
    {
      UINT UAVSize = AlignForUavCounter((UINT)(W * H * Render->CreationParams.MaxTransparentDepth * sizeof(GDRGPUOITNode)));
      CounterOffset = UAVSize;
      
      Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Buffer({ UAVSize + sizeof(UINT) }, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        OITNodesPoolGPUData);
      OITNodesPoolGPUData.Resource->SetName(L"OIT Nodes Pool");

      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
      uavDesc.Format = DXGI_FORMAT_UNKNOWN;
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
      uavDesc.Buffer.FirstElement = 0;
      uavDesc.Buffer.NumElements = (UINT)(W * H * Render->CreationParams.MaxTransparentDepth);
      uavDesc.Buffer.StructureByteStride = sizeof(GDRGPUOITNode);
      uavDesc.Buffer.CounterOffsetInBytes = UAVSize;
      uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

      Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(OITNodesPoolGPUData.Resource, OITNodesPoolGPUData.Resource, &uavDesc, NodesPoolUAVCPUDescriptor);

      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Buffer.NumElements = (UINT)(W * H * Render->CreationParams.MaxTransparentDepth);
      srvDesc.Buffer.StructureByteStride = sizeof(GDRGPUOITNode);
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(OITNodesPoolGPUData.Resource, &srvDesc, NodesPoolSRVCPUDescriptor);
    }
  }
  else
  {
    pCommandList->CopyBufferRegion(
      OITNodesPoolGPUData.Resource,
      CounterOffset,
      Render->DrawCommandsSystem->CommandsUAVReset.Resource,
      0,
      sizeof(UINT));
  }
}

void gdr::oit_transparency_subsystem::UpdateResourceState(ID3D12GraphicsCommandList* pCommandList, bool IsRender)
{
  if (IsRender)
  {
    // Transit state to needed one
    Render->GetDevice().TransitResourceState(
      pCommandList,
      OITTexture.Resource,
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    UINT Values[4] = { NONE_INDEX, NONE_INDEX, NONE_INDEX, NONE_INDEX };
    D3D12_RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = W;
    rect.bottom = H;

    pCommandList->ClearUnorderedAccessViewUint(TextureUAVGPUDescriptor, D3DDynamicHeap->GetCPUDescriptorHandleForHeapStart(), OITTexture.Resource, Values, 1, &rect);

    // Transit state to needed one
    Render->GetDevice().TransitResourceState(
      pCommandList,
      OITNodesPoolGPUData.Resource,
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  }
  else
  {
    // Transit state to needed one
    Render->GetDevice().TransitResourceState(
      pCommandList,
      OITTexture.Resource,
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    // Transit state to needed one
    Render->GetDevice().TransitResourceState(
      pCommandList,
      OITNodesPoolGPUData.Resource,
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
  }
}

gdr::oit_transparency_subsystem::~oit_transparency_subsystem()
{
  D3D_RELEASE(D3DDynamicHeap);

  if (OITTexture.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(OITTexture);
  }
  if (OITNodesPoolGPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(OITNodesPoolGPUData);
  }
}