#include "shared_structures.h"

struct D3D12_DRAW_INDEXED_ARGUMENTS
{
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int BaseVertexLocation;
    uint StartInstanceLocation;
};

struct D3D12_VERTEX_BUFFER_VIEW
{
    uint2 BufferLocation;
    uint SizeInBytes;
    uint StrideInBytes;
};

struct D3D12_INDEX_BUFFER_VIEW
{
    uint2 BufferLocation;
    uint SizeInBytes;
    uint Format;
};

struct IndirectCommand
{
    ObjectIndices indices;                     // buffer with indices
    D3D12_VERTEX_BUFFER_VIEW vertexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBuffer;
    D3D12_DRAW_INDEXED_ARGUMENTS drawArguments;
    uint2 _pad1;
};

StructuredBuffer<ObjectTransform> ObjectTransformData     : register(t2);    // SRV: Data with transforms which stored per object
StructuredBuffer<IndirectCommand> inputCommands           : register(t0);    // SRV: Indirect commands
AppendStructuredBuffer<IndirectCommand> outputCommands    : register(u0);    // UAV: Processed indirect commands
cbuffer RootConstants : register (b0)                                        // RootConstant : General data
{
  ComputeRootConstants globals;
}

bool within(float a, float t, float b)
{
    return (t >= a && t <= b);
}

bool CullAABBFrustum(
    float4x4 VP,
    float4x4 transform,
    float3 minAABB,
    float3 maxAABB)
{
    // Use our min max to define eight corners
    float4 corners[8] = {
        float4(minAABB.x, minAABB.y, minAABB.z, 1.0), // x y z
        float4(maxAABB.x, minAABB.y, minAABB.z, 1.0), // X y z
        float4(minAABB.x, maxAABB.y, minAABB.z, 1.0), // x Y z
        float4(maxAABB.x, maxAABB.y, minAABB.z, 1.0), // X Y z

        float4(minAABB.x, minAABB.y, maxAABB.z, 1.0), // x y Z
        float4(maxAABB.x, minAABB.y, maxAABB.z, 1.0), // X y Z
        float4(minAABB.x, maxAABB.y, maxAABB.z, 1.0), // x Y Z
        float4(maxAABB.x, maxAABB.y, maxAABB.z, 1.0) // X Y Z
    };

    bool inside = false;

    for (int corner_idx = 0; corner_idx < 8; corner_idx++)
    {
        // Transform vertex
        float4 corner = mul(VP, mul(transform, corners[corner_idx]));

        // Check vertex against clip space bounds
        inside = inside ||
            within(-corner.w, corner.x, corner.w) &&
            within(-corner.w, corner.y, corner.w) &&
            within(0.0f, corner.z, corner.w);
    }
    return inside;
}

[numthreads(ComputeThreadBlockSize, 1, 1)]
void CS(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    // Each thread of the CS operates on one of the indirect commands.
    uint index = (groupId.x * ComputeThreadBlockSize) + groupIndex;

    // Don't attempt to access commands that don't exist if more threads are allocated
    // than commands.
    if (index < globals.commandCount)
    {
        // if not transparent - forget about it
        if (!(inputCommands[index].indices.ObjectParams & OBJECT_PARAMETER_TRANSPARENT))
          return;
        
        // if culling disabled - add.
        if (!globals.enableCulling ||
            CullAABBFrustum(
                globals.VP,
                ObjectTransformData[inputCommands[index].indices.ObjectTransformIndex].transform,
                ObjectTransformData[inputCommands[index].indices.ObjectTransformIndex].minAABB,
                ObjectTransformData[inputCommands[index].indices.ObjectTransformIndex].maxAABB))
        {
            outputCommands.Append(inputCommands[index]);
        }
    }
}