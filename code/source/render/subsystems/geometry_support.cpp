#include "p_header.h"


// default constructor
gdr::geometry_support::geometry_support(render *Rnd)
{
  Render = Rnd;
}

// Create Geometry using vertices and Indices
bool gdr::geometry_support::CreateGeometry(const vertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount)
{
  geometry geom;
  bool res = true;
  // Ñreate vertex buffer
  if (res)
  {
    res = Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({vertexCount * sizeof(vertex)}), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, geom.VertexBuffer, pVertex, vertexCount * sizeof(vertex));
  }
  if (res)
  {
    geom.VertexBufferView.BufferLocation = geom.VertexBuffer.Resource->GetGPUVirtualAddress();
    geom.VertexBufferView.StrideInBytes = sizeof(vertex);
    geom.VertexBufferView.SizeInBytes = (UINT)(vertexCount * sizeof(vertex));
  }
  // Create index buffer
  if (res)
  {
    res = Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ indexCount * sizeof(UINT32) }), D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, geom.IndexBuffer, pIndices, indexCount * sizeof(UINT32));
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
gdr::geometry_support::~geometry_support()
{
  for (auto& geom : CPUPool)
  {
    if (!geom.IsDublicated)
    {
      Render->GetDevice().ReleaseGPUResource(geom.IndexBuffer);
      Render->GetDevice().ReleaseGPUResource(geom.VertexBuffer);
    }
  }
  CPUPool.clear();
}