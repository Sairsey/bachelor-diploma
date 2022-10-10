#include "p_header.h"

gdr::globals_support::globals_support(render* Rnd)
{
  Render = Rnd;
  StoredCopy.time = 0;
  CPUData.time = 1;
  CPUData.SkyboxCubemapIndex = -1;
  CPUData.SceneExposure = 10.0;
  GPUData.Resource = nullptr;
}

void gdr::globals_support::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // if buffers are not the same
  if (memcmp(&CPUData, &StoredCopy, sizeof(GlobalData)) != 0)
  {
    if (GPUData.Resource == nullptr)
    {
      size_t CBufferGPUSize = Align(sizeof(GlobalData), (size_t)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

      Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ CBufferGPUSize }),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        GPUData,
        &CPUData,
        sizeof(GlobalData));

      GPUData.Resource->SetName(L"Globals buffer");

      Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
      cbvDesc.BufferLocation = GPUData.Resource->GetGPUVirtualAddress();
      cbvDesc.SizeInBytes = (UINT)CBufferGPUSize;
      Render->GetDevice().GetDXDevice()->CreateConstantBufferView(&cbvDesc, CPUDescriptor);
    }
    else
    {
      Render->GetDevice().UpdateBuffer(pCommandList, GPUData.Resource, &CPUData, sizeof(GlobalData));
    }

    StoredCopy = CPUData;
  }
}

gdr::globals_support::~globals_support()
{
  if (GPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
  }
}
