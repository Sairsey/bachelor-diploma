#pragma once

#include "def.h"

/* Project namespace */
namespace gdr
{

  // GPU resource representation
  struct GPUResource
  {
    ID3D12Resource* Resource = nullptr; // Resource
    D3D12MA::Allocation* Allocation = nullptr; // allocation
  };

  // Ring buffer for descriptors
  struct descriptor_ring_buffer : public ring_buffer<descriptor_ring_buffer, D3D12_GPU_DESCRIPTOR_HANDLE>
  {
    // default constructor
    descriptor_ring_buffer() : ring_buffer<descriptor_ring_buffer, D3D12_GPU_DESCRIPTOR_HANDLE>(), DescHeap(nullptr), DescSize(0) {}

    // Initialisation func
    bool Init(ID3D12Device* pDevice, UINT descCount, ID3D12DescriptorHeap* pHeap)
    {
      HRESULT hr = S_OK;

      DescHeap = pHeap;

      if (SUCCEEDED(hr))
      {
        DescSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        return ring_buffer<descriptor_ring_buffer, D3D12_GPU_DESCRIPTOR_HANDLE>::Init(descCount);
      }

      return false;
    }
    
    // Terminate function
    void Term()
    {
      ring_buffer<descriptor_ring_buffer, D3D12_GPU_DESCRIPTOR_HANDLE>::Term();

      DescHeap = nullptr;
    }

    // get GPU descriptor by index
    inline D3D12_GPU_DESCRIPTOR_HANDLE At(UINT64 idx) const
    {
      D3D12_GPU_DESCRIPTOR_HANDLE handle = DescHeap->GetGPUDescriptorHandleForHeapStart();
      handle.ptr += idx * DescSize;
      return handle;
    }

    // get CPU descriptor by index
    inline D3D12_CPU_DESCRIPTOR_HANDLE AtCPU(UINT64 idx) const
    {
      D3D12_CPU_DESCRIPTOR_HANDLE handle = DescHeap->GetCPUDescriptorHandleForHeapStart();
      handle.ptr += idx * DescSize;
      return handle;
    }

    // get Heap
    inline ID3D12DescriptorHeap* GetHeap() const { return DescHeap; }

  private:
    ID3D12DescriptorHeap* DescHeap; // Heap with descriptors
    UINT DescSize; // Descriptor Size
  };

  // Ring buffer for common data
  struct heap_ring_buffer : public ring_buffer<heap_ring_buffer, UINT8*>
  {
    // Constructor
    heap_ring_buffer() : ring_buffer<heap_ring_buffer, UINT8*>(), DataBuffer(nullptr), UploadData(nullptr) {}

    // Initialisation function
    bool Init(ID3D12Device* pDevice, D3D12_HEAP_TYPE heapType, UINT64 heapSize)
    {
      HRESULT hr = S_OK;

      // Create upload buffer
      D3D12_HEAP_PROPERTIES heapProps = {};
      heapProps.Type = heapType;

      D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
      switch (heapType)
      {
      case D3D12_HEAP_TYPE_UPLOAD:
        state = D3D12_RESOURCE_STATE_GENERIC_READ;
        break;

      case D3D12_HEAP_TYPE_READBACK:
        state = D3D12_RESOURCE_STATE_COPY_DEST;
        break;
      }

      const auto& desc = CD3DX12_RESOURCE_DESC::Buffer({ heapSize });
      D3D_CHECK(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, state, nullptr, __uuidof(ID3D12Resource), (void**)&DataBuffer));

      D3D12_RANGE range = {};
      range.Begin = 0;
      range.End = heapSize;
      D3D_CHECK(DataBuffer->Map(0, &range, (void**)&UploadData));

      if (SUCCEEDED(hr))
      {
        return ring_buffer<heap_ring_buffer, UINT8*>::Init(heapSize);
      }

      return false;
    }

    // Termination function
    void Term()
    {
      ring_buffer<heap_ring_buffer, UINT8*>::Term();

      if (UploadData != nullptr)
      {
        D3D12_RANGE range = {};
        range.Begin = 0;
        range.End = allocMaxSize;
        DataBuffer->Unmap(0, &range);
        UploadData = nullptr;
      }

      D3D_RELEASE(DataBuffer);
    }

    // get data by index 
    inline UINT8* At(UINT64 idx) const { return UploadData + idx; }
    // get buffer
    inline ID3D12Resource* GetBuffer() const { return DataBuffer; }
  private:
    ID3D12Resource* DataBuffer; // buffer with data on GPU 
    UINT8* UploadData;          // Buffer with data on CPU
  };

  // Ring buffer for Queries
  struct query_ring_buffer : public ring_buffer<query_ring_buffer, std::function<void(UINT64)>>
  {
    // default constructor
    query_ring_buffer() : ring_buffer<query_ring_buffer, std::function<void(UINT64)>>(), GPUHeap(nullptr), PrevAllocEnd(0), HeapSize(0), CpuBuffer(), CPUData(nullptr), PrevFlashedEnd(0) {}

    // Initialisation func
    bool Init(ID3D12Device* pDevice, D3D12MA::Allocator* pMemAllocator, D3D12_QUERY_HEAP_TYPE heapType, UINT size)
    {
      HRESULT hr = S_OK;

      HeapSize = size;

      // Create query heap
      D3D12_QUERY_HEAP_DESC heapDesc = {};
      heapDesc.Type = heapType;
      heapDesc.Count = (UINT)HeapSize;
      heapDesc.NodeMask = 0;

      D3D_CHECK(pDevice->CreateQueryHeap(&heapDesc, __uuidof(ID3D12QueryHeap), (void**)&GPUHeap));

      if (SUCCEEDED(hr))
      {
        D3D12MA::ALLOCATION_DESC allocDesc;
        allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
        allocDesc.CustomPool = nullptr;
        allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
        allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;

        const auto& desc = CD3DX12_RESOURCE_DESC::Buffer(HeapSize * sizeof(UINT64));
        D3D_CHECK(pMemAllocator->CreateResource(&allocDesc, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, &CpuBuffer.Allocation, __uuidof(ID3D12Resource), (void**)&CpuBuffer.Resource));
      }

      if (SUCCEEDED(hr))
      {
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = HeapSize * sizeof(64);
        D3D_CHECK(CpuBuffer.Resource->Map(0, &range, (void**)&CPUData));
      }

      if (SUCCEEDED(hr))
      {
        Callbacks.resize(HeapSize);

        return ring_buffer<query_ring_buffer, std::function<void(UINT64)>>::Init(HeapSize);
      }

      return false;
    }

    // Termination func
    void Term()
    {
      if (CPUData != nullptr)
      {
        assert(CpuBuffer.Resource != nullptr);

        CpuBuffer.Resource->Unmap(0, nullptr);

        CPUData = nullptr;
      }

      D3D_RELEASE(CpuBuffer.Resource);
      D3D_RELEASE(CpuBuffer.Allocation);

      D3D_RELEASE(GPUHeap);
    }

    // get function by index
    inline std::function<void(UINT64)>& At(UINT64 idx) { return Callbacks[idx]; }

    // Add fence
    void AddPendingFence(UINT64 fenceValue)
    {
      if (!IsEmpty())
      {
        PrevAllocEnd = allocEnd;

        ring_buffer<query_ring_buffer, std::function<void(UINT64)>>::AddPendingFence(fenceValue);
      }
    }

    // sync fence
    void FlashFenceValue(UINT64 fenceValue)
    {
      while (!pendingFences.empty() && pendingFences.front().fenceValue <= fenceValue)
      {
        UINT64 flashedValue = pendingFences.front().allocEnd;
        if (PrevFlashedEnd < flashedValue)
        {
          for (UINT64 i = PrevFlashedEnd; i < flashedValue; i++)
          {
            if (Callbacks[i] != nullptr)
            {
              Callbacks[i](CPUData[i]);
              Callbacks[i] = std::function<void(UINT64)>();
            }
          }
        }
        else
        {
          for (UINT64 i = PrevFlashedEnd; i < allocMaxSize; i++)
          {
            if (Callbacks[i] != nullptr)
            {
              Callbacks[i](CPUData[i]);
              Callbacks[i] = std::function<void(UINT64)>();
            }
          }
          for (UINT64 i = 0; i < flashedValue; i++)
          {
            if (Callbacks[i] != nullptr)
            {
              Callbacks[i](CPUData[i]);
              Callbacks[i] = std::function<void(UINT64)>();
            }
          }
        }

        PrevFlashedEnd = flashedValue;

        allocStart = pendingFences.front().allocEnd;
        pendingFences.pop();
      }
    }

    // Resolve command list
    HRESULT Resolve(ID3D12GraphicsCommandList* pCommandList)
    {
      if (!IsEmpty())
      {
        std::pair<UINT64, UINT64> range0, range1;

        GetResolveRanges(range0, range1);

        if (range0.second != 0)
        {
          pCommandList->ResolveQueryData(
            GetQueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP, (UINT)range0.first, (UINT)range0.second, CpuBuffer.Resource, (UINT)range0.first * sizeof(UINT64)
          );
        }

        if (range1.second != 0)
        {
          pCommandList->ResolveQueryData(
            GetQueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP, (UINT)range1.first, (UINT)range1.second, CpuBuffer.Resource, (UINT)range1.first * sizeof(UINT64)
          );
        }
      }

      return S_OK;
    }

    /** Get query heap */
    ID3D12QueryHeap* GetQueryHeap() const { return GPUHeap; }

  private:
    // check if buffer empty
    bool IsEmpty() const
    {
      return PrevAllocEnd == allocEnd;
    }

    // Get ranges
    void GetResolveRanges(std::pair<UINT64, UINT64>& range0, std::pair<UINT64, UINT64>& range1) const
    {
      if (PrevAllocEnd < allocEnd)
      {
        range0 = std::make_pair(PrevAllocEnd, allocEnd - PrevAllocEnd);
        range1 = std::make_pair(-1, 0);
      }
      else
      {
        range0 = std::make_pair(PrevAllocEnd, allocMaxSize - PrevAllocEnd);
        range1 = std::make_pair(0, allocEnd);
      }
    }

  private:
    ID3D12QueryHeap* GPUHeap; // GPU Query heap
    UINT64 PrevAllocEnd;       // End of previous allocation
    UINT64 PrevFlashedEnd;     // End of previous flashed allocation

    GPUResource CpuBuffer;     // CPU bugger with data
    UINT64* CPUData;          // CPU data
    UINT64 HeapSize;           // size of heap

    // Callback which will be called on fences
    std::vector<std::function<void(UINT64)>> Callbacks;
  };
}