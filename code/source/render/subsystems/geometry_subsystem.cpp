#include "p_header.h"


// default constructor
gdr::geometry_subsystem::geometry_subsystem(render *Rnd)
{
  Render = Rnd;
}

// Create Geometry using vertices and Indices
bool gdr::geometry_subsystem::CreateGeometry(const GDRVertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount)
{
  geometry geom;
  bool res = true;
  // Ñreate vertex buffer
  if (res)
  {
    res = Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({vertexCount * sizeof(GDRVertex)}), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, geom.VertexBuffer, pVertex, vertexCount * sizeof(GDRVertex));
    geom.VertexBuffer.Resource->SetName(L"vertex_buffer");
  }
  if (res)
  {
    geom.VertexBufferView.BufferLocation = geom.VertexBuffer.Resource->GetGPUVirtualAddress();
    geom.VertexBufferView.StrideInBytes = sizeof(GDRVertex);
    geom.VertexBufferView.SizeInBytes = (UINT)(vertexCount * sizeof(GDRVertex));
  }
  // Create index buffer
  if (res)
  {
    res = Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ indexCount * sizeof(UINT32) }), D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, geom.IndexBuffer, pIndices, indexCount * sizeof(UINT32));
    geom.IndexBuffer.Resource->SetName(L"index_buffer");
  }
  if (res)
  {
    geom.IndexBufferView.BufferLocation = geom.IndexBuffer.Resource->GetGPUVirtualAddress();
    geom.IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    geom.IndexBufferView.SizeInBytes = (UINT)(indexCount * sizeof(UINT32));
  }
  geom.IndexCount = (UINT)indexCount;
  CPUPool.push_back(geom);
  return res;
}

// Destructor
gdr::geometry_subsystem::~geometry_subsystem()
{
  for (auto& geom : CPUPool)
  {
    Render->GetDevice().ReleaseGPUResource(geom.IndexBuffer);
    Render->GetDevice().ReleaseGPUResource(geom.VertexBuffer);
  }
  CPUPool.clear();
}