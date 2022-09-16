#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  // vertex representation struct
  struct vertex
  {
    mth::vec3f Pos;
    mth::vec3f Normal;
    mth::vec2f UV;
  };

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

    bool IsDublicated = false;// helps destructor do the right thing
  };

  /* Geometry support subsystem class */
  class geometry_support
  {
    private:
      // pointer to Render
      render *Render;
    public:
      // vector which contains all geometries
      std::vector<geometry> CPUPool;

      // default constructor
      geometry_support(render *Rnd);

      // Create Geometry using vertices and Indices
      bool CreateGeometry(const vertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount);

      // Destructor
      ~geometry_support();
  };
}