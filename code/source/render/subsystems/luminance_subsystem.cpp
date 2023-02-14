#include "p_header.h"

gdr::luminance_subsystem::luminance_subsystem(render* Rnd)
{
  Render = Rnd;
  GPUData.Resource = nullptr;
}

void gdr::luminance_subsystem::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  if (GPUData.Resource == nullptr)
  {
    size_t CBufferGPUSize = Align(sizeof(GDRGPULuminanceVariables), (size_t)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

    Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ CBufferGPUSize }, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      GPUData,
      nullptr,
      sizeof(GDRGPUGlobalData));

    GPUData.Resource->SetName(L"Luminance buffer");

    Render->GetDevice().AllocateStaticDescriptors(2, CPUDescriptor, GPUDescriptor);

    // set UAV for it
    {
      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
      uavDesc.Format = DXGI_FORMAT_UNKNOWN;
      uavDesc.Buffer.CounterOffsetInBytes = 0;
      uavDesc.Buffer.NumElements = 1;
      uavDesc.Buffer.StructureByteStride = sizeof(GDRGPULuminanceVariables);
      uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

      Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(GPUData.Resource, nullptr, &uavDesc, CPUDescriptor);
    }

    // set CBV for it
    {
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
      cbvDesc.BufferLocation = GPUData.Resource->GetGPUVirtualAddress();
      cbvDesc.SizeInBytes = (UINT)CBufferGPUSize;

      D3D12_CPU_DESCRIPTOR_HANDLE descr = CPUDescriptor;
      descr.ptr += Render->GetDevice().GetSRVDescSize();
      Render->GetDevice().GetDXDevice()->CreateConstantBufferView(&cbvDesc, descr);
    }
  }
}

void gdr::luminance_subsystem::UpdateResourceState(ID3D12GraphicsCommandList* pCommandList, bool IsRender)
{
  if (IsRender && !IsInited)
  {
    // Transit state to needed one
    Render->GetDevice().TransitResourceState(
      pCommandList,
      GPUData.Resource,
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    IsInited = true;
  }
}

gdr::luminance_subsystem::~luminance_subsystem()
{
  if (GPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
  }
  IsInited = false;
}
