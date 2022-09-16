#include "p_header.h"

gdr::materials_support::materials_support(render* Rnd)
{
  Render = Rnd;
  GPUData.Resource = nullptr;
}

ObjectMaterial gdr::materials_support::DefaultMaterial(void)
{
  ObjectMaterial mat;
  mat.Ka = mth::vec3f(0.f, 0.f, 0.f);
  mat.Kd = mth::vec3f(0.7f, 0.7f, 0.7f);
  mat.Ks = mth::vec3f(0.1f, 0.1f, 0.1f);
  mat.Ph = 30.0f;
  mat.KaMapIndex = -1;
  mat.KdMapIndex = -1;
  mat.KsMapIndex = -1;
  return mat;
}

void gdr::materials_support::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // if buffers are not the same
  if (CPUData.size() != StoredCopy.size() ||
    memcmp(&CPUData[0], &StoredCopy[0], sizeof(ObjectMaterial) * CPUData.size()) != 0)
  {

    if (GPUData.Resource == nullptr)
    {
      Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ sizeof(ObjectMaterial) * CPUData.size() }),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        GPUData,
        &CPUData[0],
        sizeof(ObjectMaterial) * CPUData.size());

      Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Buffer.NumElements = (UINT)CPUData.size();
      srvDesc.Buffer.StructureByteStride = sizeof(ObjectMaterial);
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(GPUData.Resource, &srvDesc, CPUDescriptor);
    }
    else
    {
      Render->GetDevice().UpdateBuffer(pCommandList, GPUData.Resource, &CPUData[0], sizeof(ObjectMaterial) * CPUData.size());
    }

    StoredCopy = CPUData;
  }
}

gdr::materials_support::~materials_support()
{
  if (GPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
  }
}
