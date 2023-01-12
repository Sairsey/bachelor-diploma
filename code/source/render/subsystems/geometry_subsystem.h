#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Geometry representation struct
  struct geometry
  {
    // Vertex Buffer
    GPUResource VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

    // Index Buffer
    GPUResource IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;

    // Amount of indices
    UINT IndexCount = 0;
  };

  /* Geometry support subsystem class */
  class geometry_subsystem
  {
    private:
      // pointer to Render
      render *Render;
    public:
      // vector which contains all geometries
      std::vector<geometry> CPUPool;

      // default constructor
      geometry_subsystem(render *Rnd);

      // Create Geometry using vertices and Indices
      bool CreateGeometry(const GDRVertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount);

      // Destructor
      ~geometry_subsystem();
  };
}