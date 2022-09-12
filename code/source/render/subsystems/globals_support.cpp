#include "p_header.h"

gdr::globals_support::globals_support(render* Rnd)
{
  Render = Rnd;
  StoredCopy.time = 0;
  CPUData.time = 1;
  GPUData.Resource = nullptr;
}

void gdr::globals_support::UpdateGPUData(void)
{
  // if buffers are not the same
  if (memcmp(&CPUData, &StoredCopy, sizeof(GlobalData)) != 0)
  {
    ID3D12GraphicsCommandList *uploadCommandList;
    Render->GetDevice().BeginUploadCommandList(&uploadCommandList);
    PROFILE_BEGIN(uploadCommandList, "Update globals");

    if (GPUData.Resource == nullptr)
    {
      Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ sizeof(GlobalData) }),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        GPUData,
        &CPUData,
        sizeof(GlobalData));

      Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
      cbvDesc.BufferLocation = GPUData.Resource->GetGPUVirtualAddress();
      cbvDesc.SizeInBytes = Align(sizeof(GlobalData), (size_t)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
      Render->GetDevice().GetDXDevice()->CreateConstantBufferView(&cbvDesc, CPUDescriptor);
    }
    else
    {
      Render->GetDevice().UpdateBuffer(uploadCommandList, GPUData.Resource, &CPUData, sizeof(GlobalData));
    }

    PROFILE_END(uploadCommandList);
    Render->GetDevice().CloseUploadCommandList();
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
