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

StructuredBuffer<ObjectTransform> ObjectTransformData           : register(t2);    // SRV: Data with transforms which stored per object
StructuredBuffer<IndirectCommand> inputCommands                 : register(t0);    // SRV: Indirect commands
AppendStructuredBuffer<IndirectCommand> opaqueCommands          : register(u0);    // UAV: Processed indirect commands
AppendStructuredBuffer<IndirectCommand> opaqueCulledCommands    : register(u1);    // UAV: Processed indirect commands
AppendStructuredBuffer<IndirectCommand> transCommands           : register(u3);    // UAV: Processed indirect commands
Texture2D HierDepth : register(t3);    // SRV: Data with transforms which stored per object
SamplerState SamplePoint: register(s0);

cbuffer RootConstants : register (b0)                                        // RootConstant : General data
{
  ComputeRootConstants globals;
}

bool within(float a, float t, float b)
{
    return (t >= a && t <= b);
}

bool CullOcclusion(float4 screenSpaceCorners[8])
{
  float4 corner = screenSpaceCorners[0] / screenSpaceCorners[0].w;
  float depth = corner.z;
  float2 minCSBB = corner.xy;
  float2 maxCSBB = minCSBB;
  for (int i = 0; i < 8; i++)
  {
    corner = screenSpaceCorners[i] / screenSpaceCorners[i].w;
    depth = min(corner.z, depth);
    minCSBB.x = min(minCSBB.x, corner.x);
    minCSBB.y = min(minCSBB.y, corner.y);
    maxCSBB.x = max(maxCSBB.x, corner.x);
    maxCSBB.y = max(maxCSBB.y, corner.y);
  }

  uint width, height, mipCount;
  HierDepth.GetDimensions(0, width, height, mipCount);

  float2 minSSBB = minCSBB * float2(0.5, -0.5) + (0.5).xx;
  float2 maxSSBB = maxCSBB * float2(0.5, -0.5) + (0.5).xx;

  // clamp values so they are
  maxSSBB = max(maxSSBB, (0).xx);
  minSSBB = max(minSSBB, (0).xx);
  maxSSBB = min(maxSSBB, (1).xx);
  minSSBB = min(minSSBB, (1).xx);

  float2 WH = (maxSSBB - minSSBB) * float2(width, height);
  float param = min(ceil(log2(max(WH.x, WH.y))), mipCount - 1);
  
  float HierDepth1 = HierDepth.SampleLevel(SamplePoint, float2(minSSBB.x, minSSBB.y), param);
  float HierDepth2 = HierDepth.SampleLevel(SamplePoint, float2(maxSSBB.x, minSSBB.y), param);
  float HierDepth3 = HierDepth.SampleLevel(SamplePoint, float2(minSSBB.x, maxSSBB.y), param);
  float HierDepth4 = HierDepth.SampleLevel(SamplePoint, float2(maxSSBB.x, maxSSBB.y), param);
  
  float HierDepthColor = max(max(HierDepth1, HierDepth2), max(HierDepth3, HierDepth4));
  return HierDepthColor - depth >= 0;
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

    for (int corner_idx = 0; corner_idx < 8; corner_idx++)
    {
        corners[corner_idx] = mul(VP, mul(transform, corners[corner_idx]));
        corners[corner_idx] /= abs(corners[corner_idx].w);
    }

    bool LeftPlaneResult = true;
    bool RightPlaneResult = true;
    bool TopPlaneResult = true;
    bool BottomPlaneResult = true;
    bool FarPlaneResult = true;
    bool NearPlaneResult = true;
    bool IsIntersectNear = false;

    for (int corner_idx = 0; corner_idx < 8; corner_idx++)
    {
        LeftPlaneResult = LeftPlaneResult && corners[corner_idx].x <= -1;
        RightPlaneResult = RightPlaneResult && corners[corner_idx].x >= 1;

        BottomPlaneResult = BottomPlaneResult && corners[corner_idx].y <= -1;
        TopPlaneResult = TopPlaneResult && corners[corner_idx].y >= 1;

        FarPlaneResult = FarPlaneResult && corners[corner_idx].z >= 1;
        NearPlaneResult = NearPlaneResult && corners[corner_idx].z <= 0;
        IsIntersectNear = IsIntersectNear || corners[corner_idx].z <= 0;
    }

    bool inside = !(LeftPlaneResult || RightPlaneResult || TopPlaneResult || BottomPlaneResult || FarPlaneResult || NearPlaneResult);
    inside = inside && (IsIntersectNear || CullOcclusion(corners));
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
        IndirectCommand command = inputCommands[index];
        // save index
        command.indices.ObjectIndex = index;
        bool visible = !globals.enableCulling ||
          CullAABBFrustum(
            globals.VP,
            ObjectTransformData[command.indices.ObjectTransformIndex].transform,
            ObjectTransformData[command.indices.ObjectTransformIndex].minAABB,
            ObjectTransformData[command.indices.ObjectTransformIndex].maxAABB);
        bool opaque = !(command.indices.ObjectParams & OBJECT_PARAMETER_TRANSPARENT);

        if (opaque)
        {
          opaqueCommands.Append(command);
        }

        if (opaque && visible)
        {
          opaqueCulledCommands.Append(command);
        }

        if (!opaque && visible)
        {
          transCommands.Append(command);
        }
    }
}