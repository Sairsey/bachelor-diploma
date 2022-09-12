#include "p_header.h"

gdr::transforms_support::transforms_support(render* Rnd)
{
  Render = Rnd;
  GPUData.Resource = nullptr;
}

void gdr::transforms_support::UpdateGPUData(void)
{
  // if buffers are not the same
  if (CPUData.size() != StoredCopy.size() ||
    memcmp(&CPUData[0], &StoredCopy[0], sizeof(ObjectTransform) * CPUData.size()) != 0)
  {
    ID3D12GraphicsCommandList* uploadCommandList;
    Render->GetDevice().BeginUploadCommandList(&uploadCommandList);
    PROFILE_BEGIN(uploadCommandList, "Update transforms");

    for (int i = 0; i < CPUData.size(); i++)
    {
      CPUData[i].transformInversedTransposed = CPUData[i].transform.Inversed().Transposed();
    }

    if (GPUData.Resource == nullptr)
    {
      Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ sizeof(ObjectTransform) * CPUData.size() }),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        GPUData,
        &CPUData[0],
        sizeof(ObjectTransform) * CPUData.size());

      Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Buffer.NumElements = CPUData.size();
      srvDesc.Buffer.StructureByteStride = sizeof(ObjectTransform);
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(GPUData.Resource, &srvDesc, CPUDescriptor);
    }
    else
    {
      Render->GetDevice().UpdateBuffer(uploadCommandList, GPUData.Resource, &CPUData[0], sizeof(ObjectTransform) * CPUData.size());
    }

    PROFILE_END(uploadCommandList);
    Render->GetDevice().CloseUploadCommandList();
    StoredCopy = CPUData;
  }
}

gdr::transforms_support::~transforms_support()
{
  if (GPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
  }
}
