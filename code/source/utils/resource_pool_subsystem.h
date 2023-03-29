#include "def.h"

/* Project namespace */
namespace gdr
{
  // Subsystem of holding StoredType in Pool on GPU and CPU
  template<typename StoredType, gdr_index_types Type, int ChunkSize>
  class resource_pool_subsystem
  {
  protected:
    struct resource_pool_record
    {
      bool IsAlive;           // if False -> this resource was deleted
      bool IsNeedToBeDeleted; // if True -> We should delete on next frame
      int ReferenceCount = 1;
    };

    render* Render; // pointer on Render

    // GPU Resource Allocation
    GPUResource GPUData;

    // Resource Descriptors
    bool DescriptorsAllocated = false;
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;

    // Buffer splitted in chunks.
    std::vector<resource_pool_record> PoolRecords; // records used for "Add", "Remove" and "Get"
    std::vector<StoredType> CPUData;
    size_t StoredSize;
    const int CHUNK_SIZE = ChunkSize;
    std::vector<bool> ChunkMarkings;

    /// INIT PARAMS
    // Value for debuggers to identify this pool
    std::wstring ResourceName = L"Template resource pool";
    // Resource states used
    D3D12_RESOURCE_STATES UsedResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

    // Job to do before update
    virtual void BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList) {};
    // Job to do after update
    virtual void AfterUpdateJob(ID3D12GraphicsCommandList* pCommandList) {};
    // Job to do after update of resource state
    virtual void AfterResourceStateUpdateJob(ID3D12GraphicsCommandList* pCommandList, bool IsRender) {};
    // Job to do before removing
    virtual void BeforeRemoveJob(gdr_index index) {};

    // Helper funtion to create GPUResource and init everything
    void CreateResource();
    // Helper funtion to delete GPUResource
    void DeleteResource();
  public:
    // Constructor
    resource_pool_subsystem(render* Rnd);

    // Update GPU Data
    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    // Update state of resource. If "IsRender" is true, then update to UsedResourceState. Otherwise to D3D12_RESOURCE_STATE_COPY_DEST
    void UpdateResourceState(ID3D12GraphicsCommandList* pCommandList, bool IsRender);

    // Add element in Pool
    gdr_index Add();

    // Increase refCount of current resource
    void IncreaseReferenceCount(gdr_index index)
    {
      if (IsExist(index))
        PoolRecords[index].ReferenceCount++;
    }

    // Remove element from Pool
    void Remove(gdr_index index);

    // Get element the way it can be edited
    StoredType &GetEditable(gdr_index index);

    // Get element the way it cannot be edited
    const StoredType& Get(gdr_index index) const;

    // Check if element by this index exists
    bool IsExist(gdr_index index) const;

    // Get GPU resource
    GPUResource& GetGPUResource() {return GPUData;};

    // Get Size
    size_t AllocatedSize() {return CPUData.size();}

    // Mark specific chunk to update
    void MarkChunkByElementIndex(gdr_index index);

    // Destructor 
    ~resource_pool_subsystem(void);
  };

  // Subsystem of holding StoredType in Pool on GPU and CPU
  template<typename StoredType, gdr_index_types Type>
  class resource_pool_subsystem<StoredType, Type, 0>
  {
  protected:
    struct resource_pool_record
    {
      bool IsAlive;           // if False -> this resource was deleted
      bool IsNeedToBeDeleted; // if True -> We should delete on next frame
      int ReferenceCount = 1;
    };

    render* Render; // pointer on Render

    // Buffer splitted in chunks.
    std::vector<resource_pool_record> PoolRecords; // records used for "Add", "Remove" and "Get"
    std::vector<StoredType> CPUData;

    // Job to do before update
    virtual void BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList) {};

    // Job to do after update
    virtual void AfterUpdateJob(ID3D12GraphicsCommandList* pCommandList) {};

    // Job to do before removing
    virtual void BeforeRemoveJob(gdr_index index) {};
  public:
    // Constructor
    resource_pool_subsystem(render* Rnd);

    // Add element in Pool
    gdr_index Add();

    // Increase refCount of current resource
    void IncreaseReferenceCount(gdr_index index)
    {
      if (IsExist(index))
        PoolRecords[index].ReferenceCount++;
    }

    // Remove element from Pool
    void Remove(gdr_index index);

    // Update GPU Data
    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    // Get element the way it can be edited
    StoredType& GetEditable(gdr_index index);

    // Get element the way it cannot be edited
    const StoredType& Get(gdr_index index) const;

    // Check if element by this index exists
    bool IsExist(gdr_index index) const;

    // Get Size
    size_t AllocatedSize() { return CPUData.size(); }

    // Destructor 
    ~resource_pool_subsystem(void);
  };
}

#include "resource_pool_subsystem.hpp"
#include "resource_pool_subsystem_no_gpu.hpp"