#include "shared/cpu_gpu_shared.h"

cbuffer RootConstants : register (GDRGPUComputeGlobalDataConstantBufferSlot)  // RootConstant : General data
{
  GDRGPUComputeGlobals globals;
}

StructuredBuffer<GDRGPUIndirectCommand> AllCommands : register(GDRGPUAllCommandsPoolSlot); // SRV: input Indirect commands
StructuredBuffer<GDRGPUObjectTransform> ObjectTransformPool : register(GDRGPUObjectTransformPoolSlot);  // SRV: Data with transforms which stored per object
AppendStructuredBuffer<GDRGPUIndirectCommand> OpaqueAllCommands : register(GDRGPUOpaqueAllCommandsPoolSlot);    // UAV: Processed indirect commands
AppendStructuredBuffer<GDRGPUIndirectCommand> TransparentAllCommands : register(GDRGPUTransparentAllCommandsPoolSlot);    // UAV: Processed indirect commands
AppendStructuredBuffer<GDRGPUIndirectCommand> OpaqueFrustumCulledCommands : register(GDRGPUOpaqueFrustumCommandsPoolSlot);    // UAV: Processed indirect commands

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
    return inside;
}

[numthreads(GDRGPUComputeThreadBlockSize, 1, 1)]
void CS(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    // Each thread of the CS operates on one of the indirect commands.
    uint index = (groupId.x * GDRGPUComputeThreadBlockSize) + groupIndex;

    // Don't attempt to access commands that don't exist if more threads are allocated
    // than commands.
    if (index < globals.commandsCount)
    {
        GDRGPUIndirectCommand command = AllCommands[index];
        
        // save index
        command.Indices.ObjectIndex = index;

        bool visible = !globals.frustumCulling ||
          CullAABBFrustum(
            globals.VP,
            ObjectTransformPool[command.Indices.ObjectTransformIndex].Transform,
            ObjectTransformPool[command.Indices.ObjectTransformIndex].minAABB,
            ObjectTransformPool[command.Indices.ObjectTransformIndex].maxAABB);
        bool opaque = !(command.Indices.ObjectParamsMask & OBJECT_PARAMETER_TRANSPARENT);

        if (opaque)
          OpaqueAllCommands.Append(command);
        else
          TransparentAllCommands.Append(command);
        
        if (opaque && visible)
          OpaqueFrustumCulledCommands.Append(command);
    }
}