#include "p_header.h"

gdr::globals_subsystem::globals_subsystem(render* Rnd)
{
  Render = Rnd;
  StoredCopy.Time = 0;
  CPUData.Time = 1;
  GPUData.Resource = nullptr;
  SetDefault();
}

void gdr::globals_subsystem::SetDefault(void)
{
  CPUData.VP = mth::matr4f::Identity();
  CPUData.CameraPos = mth::vec3f(0);
  CPUData.Time = 0;
  CPUData.DeltaTime = 0;
  CPUData.Width = 100;
  CPUData.Height = 100;
  CPUData.LightsAmount = 1;
  CPUData.SkyboxIndex = NONE_INDEX;
}

void gdr::globals_subsystem::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // if buffers are not the same
  if (memcmp(&CPUData, &StoredCopy, sizeof(GDRGPUGlobalData)) != 0)
  {
    if (GPUData.Resource == nullptr)
    {
      size_t CBufferGPUSize = Align(sizeof(GDRGPUGlobalData), (size_t)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

      Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ CBufferGPUSize }),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        GPUData,
        &CPUData,
        sizeof(GDRGPUGlobalData));

      GPUData.Resource->SetName(L"Globals buffer");

      Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
      cbvDesc.BufferLocation = GPUData.Resource->GetGPUVirtualAddress();
      cbvDesc.SizeInBytes = (UINT)CBufferGPUSize;
      Render->GetDevice().GetDXDevice()->CreateConstantBufferView(&cbvDesc, CPUDescriptor);
      Render->GetDevice().TransitResourceState(
        pCommandList,
        GPUData.Resource,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
    }
    else
    {
      Render->GetDevice().TransitResourceState(
        pCommandList,
        GPUData.Resource,
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
      Render->GetDevice().UpdateBuffer(pCommandList, GPUData.Resource, &CPUData, sizeof(GDRGPUGlobalData));
      Render->GetDevice().TransitResourceState(
        pCommandList,
        GPUData.Resource,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
    }

    StoredCopy = CPUData;
  }
}

gdr::globals_subsystem::~globals_subsystem()
{
  if (GPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
  }
}
