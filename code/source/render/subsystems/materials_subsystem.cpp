#include "p_header.h"

gdr::materials_subsystem::materials_subsystem(render* Rnd)
{
  Render = Rnd;
  GPUData.Resource = nullptr;
  ChunkMarkings.resize(0);
  StoredSize = 0;
  // add some default material
  AddElementInPool();
  CPUData[0].ShadeType = MATERIAL_SHADER_COLOR;
  GDRGPUMaterialColorGetColor(CPUData[0]) = mth::vec3f(1, 0, 1);
}

void gdr::materials_subsystem::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  // if buffers are not the same size - recreate everything
  if (CPUData.size() > StoredSize)
  {
    if (GPUData.Resource != nullptr)
    {
        Render->GetDevice().ReleaseGPUResource(GPUData);
        GPUData.Resource = nullptr;
    }

    Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ sizeof(GDRGPUMaterial) * CPUData.size() }),
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      GPUData,
      &CPUData[0],
      sizeof(GDRGPUMaterial) * CPUData.size());
    GPUData.Resource->SetName(L"Materials Pool");

    Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = (UINT)CPUData.size();
    srvDesc.Buffer.StructureByteStride = sizeof(GDRGPUMaterial);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    Render->GetDevice().GetDXDevice()->CreateShaderResourceView(GPUData.Resource, &srvDesc, CPUDescriptor);
    ChunkMarkings.resize(ceil(1.0 * CPUData.size() * sizeof(GDRGPUMaterial) / CHUNK_SIZE), false);

    StoredSize = CPUData.size();
  }
  else
  {
    for (int i = 0; i < ChunkMarkings.size(); i++)
    {
      if (ChunkMarkings[i]) // if some chunk changed
      {
        // probably chunks after this are needed in update as well
        int chunk_amount = 1;
        while (i + chunk_amount < ChunkMarkings.size() && ChunkMarkings[i + chunk_amount])
            chunk_amount++;

        int source_offset = i * CHUNK_SIZE;
        int dataSize = min(sizeof(GDRGPUMaterial) * CPUData.size() - source_offset, CHUNK_SIZE * chunk_amount); // Real size of chunk

        Render->GetDevice().UpdateBufferOffset(pCommandList, GPUData.Resource, source_offset, (byte*)&CPUData[0] + source_offset, dataSize); // update only 1 chunk
        for (int j = 0; j < chunk_amount; j++)
            ChunkMarkings[i + j] = false;
      }
    }
  }
}

void gdr::materials_subsystem::MarkChunkByMaterialIndex(gdr_index index)
{
    size_t chunk_index = floor(1.0 * ((byte*)&CPUData[index] - (byte*)&CPUData[0]) / CHUNK_SIZE);
    if (chunk_index < ChunkMarkings.size() && chunk_index >= 0)
        ChunkMarkings[chunk_index] = true;
}

gdr::materials_subsystem::~materials_subsystem()
{
  if (GPUData.Resource != nullptr)
  {
    Render->GetDevice().ReleaseGPUResource(GPUData);
  }
}
