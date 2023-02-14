#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  extern D3D12_INPUT_ELEMENT_DESC defaultInputElementLayout[6];

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
  class geometry_subsystem : public resource_pool_subsystem<geometry, 0>
  {
    protected:
      // Remove Geometry by index
      void BeforeRemoveJob(gdr_index index) override;
    public:
      // default constructor
      geometry_subsystem(render* Rnd) : resource_pool_subsystem(Rnd) {}

      // If we try to create geometry without params -> we should error about it
      gdr_index Add()
      {
        GDR_FAILED("You cannot create a geometry without parameters");
        return NONE_INDEX;
      }

      // Create Geometry using vertices and Indices
      gdr_index Add(const GDRVertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount);

      // Destructor
      ~geometry_subsystem();
  };
}