#pragma once

#include <queue>

/* Project namespace */
namespace gdr
{

  // Enum of RingBuffer Results
  enum class ring_buffer_result
  {
    Ok = 0,             // Space allocated
    NoRoom,             // No room for data of such size (it is needed to free some data)
    AllocTooLarge       // Retrieved allocation is too large and cannot be fit into buffer of current size
  };

  template <typename T>
  inline T Align(const T& value, const T& alignment)
  {
    return ((value + alignment - 1) / alignment) * alignment;
  }

  // Ring buffer implementation struct
  template <typename T, typename AllocType>
  struct ring_buffer
  {
    // Default constuctor
    ring_buffer() : allocStart(0), allocEnd(0), allocMaxSize(0) {}
    /* Initialization function
     * ARGUMENTS: (UINT64) - maximum allocation size
     * RETURNS: (bool) return true if init success and false otherwise 
     */
    bool Init(UINT64 _allocMaxSize)
    {
      allocMaxSize = _allocMaxSize;

      return true;
    }

    /* De-initialization function
     * ARGUMENTS: None
     * RETURNS: None
     */
    void Term()
    {
      allocStart = allocEnd = allocMaxSize = 0;
    }

    /* Allocation function
     * ARGUMENTS:
     *    - Amount of bytes to allocate
     *        (UINT64) allocSize
     *    - Alignment
     *        (UINT) align
     * RETURNS:
     *    - Result of allocation 
     *        (RingBufferResult)
     *    - Address of allocation
     *       (UINT64&) allocStartOffset
     *    - Allocation type
     *       (AllocType&) allocation
     */
    ring_buffer_result Alloc(UINT64 allocSize, UINT64& allocStartOffset, AllocType& allocation, UINT align)
    {
      UINT64 alignedAllocEnd = Align(allocEnd, (UINT64)align);

      if (allocSize > allocMaxSize)
      {
        return ring_buffer_result::AllocTooLarge;
      }

      if (allocStart <= allocEnd)
      {
        UINT64 allocSizeLeft = allocMaxSize - alignedAllocEnd;
        if (allocSize > allocSizeLeft) // we have some troubles
        {
          if (allocStart < allocSize) // we have no space left
            return ring_buffer_result::NoRoom;
          alignedAllocEnd = 0;
        }
      }
      if (allocStart > allocEnd)
      {
        if (allocSize > allocStart - alignedAllocEnd)
        {
          return ring_buffer_result::NoRoom;
        }
      }

      allocStartOffset = alignedAllocEnd;

      allocation = static_cast<T*>(this)->At(alignedAllocEnd);

      allocEnd = alignedAllocEnd + allocSize;

      return ring_buffer_result::Ok;
    }

    /* Store Pending Fence function
     * ARGUMENTS:
     *    - Fence value
     *        (UINT64) fenceValue
     * RETURNS: None.
     */
    void AddPendingFence(UINT64 fenceValue)
    {
      pendingFences.push({ fenceValue, allocEnd });
    }

    /* Remove all pending fences up to specific fence function
     * ARGUMENTS:
     *    - Fence value
     *        (UINT64) fenceValue
     * RETURNS: None.
     */
    void FlashFenceValue(UINT64 fenceValue)
    {
      while (!pendingFences.empty() && pendingFences.front().fenceValue <= fenceValue)
      {
        allocStart = pendingFences.front().allocEnd;
        pendingFences.pop();
      }
    }

  protected:
    /* Inner pending fence struct */
    struct PendingFence
    {
      UINT64 fenceValue; // fence value
      UINT64 allocEnd; // Current allocation end
    };

  protected:
    UINT64 allocStart;   // start of used data
    UINT64 allocEnd;     // End of used data
    UINT64 allocMaxSize; // Allocated size

    // queue of pending fences
    std::queue<PendingFence> pendingFences;
  };
}