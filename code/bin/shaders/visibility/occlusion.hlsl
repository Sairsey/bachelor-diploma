#include "shared/cpu_gpu_shared.h"

cbuffer RootConstants : register (GDRGPUComputeGlobalDataConstantBufferSlot)  // RootConstant : General data
{
  GDRGPUComputeGlobals globals;
}

StructuredBuffer<GDRGPUObjectTransform> ObjectTransformPool : register(GDRGPUObjectTransformPoolSlot);  // SRV: Data with transforms which stored per object
StructuredBuffer<GDRGPUIndirectCommand> AllCommands : register(GDRGPUAllCommandsPoolSlot); // SRV: input Indirect commands
Texture2D HierDepth : register(GDRGPUHierDepthSlot);
AppendStructuredBuffer<GDRGPUIndirectCommand> OpaqueCulledCommands : register(GDRGPUOpaqueCulledCommandsPoolSlot);    // UAV: Processed indirect commands
AppendStructuredBuffer<GDRGPUIndirectCommand> TransparentCulledCommands : register(GDRGPUTransparentsCulledCommandsPoolSlot);    // UAV: Processed indirect commands
SamplerState SamplePoint: register(s0);

bool within(float a, float t, float b)
{
  return (t >= a && t <= b);
}

bool CullOcclusion(float4 screenSpaceCorners[8])
{
  if (!globals.occlusionCulling)
    return true;

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
  if (!globals.frustumCulling)
    return true;
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
        if (command.IsExist)
        {
          // save index
          command.Indices.ObjectIndex = index;

          bool visible = CullAABBFrustum(
              globals.VP,
              ObjectTransformPool[command.Indices.ObjectTransformIndex].Transform,
              ObjectTransformPool[command.Indices.ObjectTransformIndex].minAABB,
              ObjectTransformPool[command.Indices.ObjectTransformIndex].maxAABB);
          bool opaque = !(command.Indices.ObjectParamsMask & OBJECT_PARAMETER_TRANSPARENT);

          if (visible)
          {
            if (opaque)
              OpaqueCulledCommands.Append(command);
            else
              TransparentCulledCommands.Append(command);
          }
        }
    }
}