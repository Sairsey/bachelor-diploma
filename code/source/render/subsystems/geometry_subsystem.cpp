#include "p_header.h"

D3D12_INPUT_ELEMENT_DESC gdr::defaultInputElementLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "BONES_ID", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "BONES_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

// Create Geometry using vertices and Indices
gdr_index gdr::geometry_subsystem::Add(const GDRVertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount)
{
  gdr_index Result = resource_pool_subsystem::Add();
  geometry& geom = GetEditable(Result);

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
  return Result;
}

// Remove Geometry by index
void gdr::geometry_subsystem::BeforeRemoveJob(gdr_index index)
{
  if (IsExist(index))
  {
    Render->GetDevice().ReleaseGPUResource(GetEditable(index).IndexBuffer);
    Render->GetDevice().ReleaseGPUResource(GetEditable(index).VertexBuffer);
  }
}

// Destructor
gdr::geometry_subsystem::~geometry_subsystem()
{
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i))
    {
      Render->GetDevice().ReleaseGPUResource(GetEditable(i).IndexBuffer);
      Render->GetDevice().ReleaseGPUResource(GetEditable(i).VertexBuffer);
    }
}