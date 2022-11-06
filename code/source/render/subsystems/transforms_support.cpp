#include "p_header.h"
#include <thread>
#include <future>


gdr::transforms_support::transforms_support(render* Rnd)
{
  Render = Rnd;
  GPUData.Resource = nullptr;
  ChunkMarkings.resize(0);
  StoredSize = 0;
}

void gdr::transforms_support::UpdateInverseTranspose(void)
{
/*
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
  */
}

void gdr::transforms_support::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // if buffers are not the same size - recreate everything
  if (CPUData.size() != StoredSize)
  {
    if (GPUData.Resource != nullptr)
    {
      Render->GetDevice().ReleaseGPUResource(GPUData);
      GPUData.Resource = nullptr;
    }
    Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ sizeof(ObjectTransform) * CPUData.size() }),
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      GPUData,
      &CPUData[0],
      sizeof(ObjectTransform) * CPUData.size());
    GPUData.Resource->SetName(L"Transfroms pool");

    Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = (UINT)CPUData.size();
    srvDesc.Buffer.StructureByteStride = sizeof(ObjectTransform);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    Render->GetDevice().GetDXDevice()->CreateShaderResourceView(GPUData.Resource, &srvDesc, CPUDescriptor);
    ChunkMarkings.resize(ceil(1.0 * CPUData.size() * sizeof(ObjectTransform) / CHUNK_SIZE), false);

    StoredSize = CPUData.size();
  }
  else
  {
    for (int i = 0; i < ChunkMarkings.size(); i++)
    {
      if (ChunkMarkings[i]) // if chunk changed
      {
        int source_offset = i * CHUNK_SIZE;
        int dataSize = min(sizeof(ObjectTransform) * CPUData.size() - source_offset, CHUNK_SIZE); // Real size of chunk      
        Render->GetDevice().UpdateBufferOffset(pCommandList, GPUData.Resource, source_offset, (byte*)&CPUData[0] + source_offset, dataSize); // update only 1 chunk
        ChunkMarkings[i] = false;
      }
    }
  }

}

void gdr::transforms_support::MarkChunkByTransformIndex(size_t index)
{
  size_t chunk_index = floor(1.0 * ((byte*)&CPUData[index] - (byte*)&CPUData[0]) / CHUNK_SIZE);
  if (chunk_index < ChunkMarkings.size())
    ChunkMarkings[chunk_index] = true;
}


gdr::transforms_support::~transforms_support()
{
  if (GPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
  }
}
