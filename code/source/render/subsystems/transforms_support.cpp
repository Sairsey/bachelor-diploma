#include "p_header.h"
#include <thread>
#include <future>


gdr::transforms_support::transforms_support(render* Rnd)
{
  Render = Rnd;
  GPUData.Resource = nullptr;
}

void gdr::transforms_support::UpdateInverseTranspose(void)
{
  const auto processor_count = std::thread::hardware_concurrency();
  std::vector<std::future<void>> f;

  for (int thread_i = 0; thread_i < processor_count; thread_i++)
  {
    f.push_back(std::async(std::launch::async, [&](int j) {
      for (int i = j; i < CPUData.size(); i += processor_count)
      {
        CPUData[i].transformInversedTransposed = CPUData[i].transform.Inversed().Transposed();
      }
    }, thread_i));
  }

  for (int thread_i = 0; thread_i < processor_count; thread_i++)
  {
    f[thread_i].wait();
  }
}

void gdr::transforms_support::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // if buffers are not the same
  if (CPUData.size() != StoredCopy.size() ||
    memcmp(&CPUData[0], &StoredCopy[0], sizeof(ObjectTransform) * CPUData.size()) != 0)
  {

    if (CPUData.size() != StoredCopy.size() && GPUData.Resource != nullptr)
    {
      Render->GetDevice().ReleaseGPUResource(GPUData);
      GPUData.Resource = nullptr;
    }

    // Now updating on GPU and only for those, who are ready to draw
    //UpdateInverseTranspose();

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
      srvDesc.Buffer.NumElements = (UINT)CPUData.size();
      srvDesc.Buffer.StructureByteStride = sizeof(ObjectTransform);
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

      GPUData.Resource->SetName(L"Transfroms pool");

      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(GPUData.Resource, &srvDesc, CPUDescriptor);
    }
    else
    {
      Render->GetDevice().UpdateBuffer(pCommandList, GPUData.Resource, &CPUData[0], sizeof(ObjectTransform) * CPUData.size());
    }

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
