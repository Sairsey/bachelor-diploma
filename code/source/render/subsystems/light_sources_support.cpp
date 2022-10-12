#include "p_header.h"

gdr::light_sources_support::light_sources_support(render* Rnd)
{
  Render = Rnd;
  GPUData.Resource = nullptr;
  Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
}

void gdr::light_sources_support::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // if buffers are not the same
  if (CPUData.size() != StoredCopy.size() ||
    memcmp(&CPUData[0], &StoredCopy[0], sizeof(LightSource) * CPUData.size()) != 0)
  {
    if (CPUData.size() != StoredCopy.size() && GPUData.Resource != nullptr)
    {
      Render->GetDevice().ReleaseGPUResource(GPUData);
      GPUData.Resource = nullptr;
    }

    if (GPUData.Resource == nullptr)
    {
      Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ sizeof(LightSource) * CPUData.size() }),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        GPUData,
        &CPUData[0],
        sizeof(LightSource) * CPUData.size());
      GPUData.Resource->SetName(L"Lights Pool");

      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Buffer.NumElements = (UINT)CPUData.size();
      srvDesc.Buffer.StructureByteStride = sizeof(LightSource);
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
      Render->GlobalsSystem->CPUData.LightsAmount = CPUData.size();
      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(GPUData.Resource, &srvDesc, CPUDescriptor);
    }
    else
    {
      Render->GetDevice().UpdateBuffer(pCommandList, GPUData.Resource, &CPUData[0], sizeof(LightSource) * CPUData.size());
    }

    StoredCopy = CPUData;
  }
}

gdr::gdr_light gdr::light_sources_support::AddDirectionalLightSource()
{
  ObjectTransform newTransform;
  newTransform.transform = mth::matr4f::Identity();
  Render->TransformsSystem->CPUData.push_back(newTransform);

  LightSource light;
  light.LightSourceType = LIGHT_SOURCE_TYPE_DIRECTIONAL;
  light.Color = mth::vec3f(1, 1, 1);
  light.ObjectTransformIndex = Render->TransformsSystem->CPUData.size() - 1;
  CPUData.push_back(light);
  return CPUData.size() - 1;
}

gdr::gdr_light gdr::light_sources_support::AddPointLightSource()
{
  ObjectTransform newTransform;
  newTransform.transform = mth::matr4f::Identity();
  Render->TransformsSystem->CPUData.push_back(newTransform);

  LightSource light;
  light.LightSourceType = LIGHT_SOURCE_TYPE_POINT;
  light.Color = mth::vec3f(1, 1, 1);
  light.ObjectTransformIndex = Render->TransformsSystem->CPUData.size() - 1;
  light.ConstantAttenuation = 1.0f;
  light.LinearAttenuation = 0.09f;
  light.QuadricAttenuation = 0.032f;
  CPUData.push_back(light);
  return CPUData.size() - 1;
}

gdr::gdr_light gdr::light_sources_support::AddSpotLightSource()
{
  ObjectTransform newTransform;
  newTransform.transform = mth::matr4f::Identity();
  Render->TransformsSystem->CPUData.push_back(newTransform);

  LightSource light;
  light.LightSourceType = LIGHT_SOURCE_TYPE_SPOT;
  light.Color = mth::vec3f(1, 1, 1);
  light.ObjectTransformIndex = Render->TransformsSystem->CPUData.size() - 1;
  light.ConstantAttenuation = 1.0f;
  light.LinearAttenuation = 0.09f;
  light.QuadricAttenuation = 0.032f;
  light.AngleInnerCone = MTH_D2R * 45.0 / 2.0;
  light.AngleOuterCone = MTH_D2R * 60.0 / 2.0;
  CPUData.push_back(light);
  return CPUData.size() - 1;
}

LightSource& gdr::light_sources_support::GetLight(gdr_light Index)
{
  return CPUData[Index];
}
ObjectTransform& gdr::light_sources_support::GetTransform(gdr_light Index)
{
  return Render->TransformsSystem->CPUData[CPUData[Index].ObjectTransformIndex];
}

gdr::light_sources_support::~light_sources_support()
{
  if (GPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
  }
}
