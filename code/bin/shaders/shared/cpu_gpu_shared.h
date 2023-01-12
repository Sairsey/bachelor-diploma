#pragma once

#ifdef __cplusplus
#include "utils/math/mth.h"
using float4x4 = mth::matr;
using float3 = mth::vec3f;
using float4 = mth::vec4f;
// C++ code
#pragma pack(push, 1)
#else
// HLSL code
#define UINT uint
#endif // __cplusplus

#include "indirect_structures.h"

// slots for all pools
#define GDRGPUGlobalDataConstantBufferSlot 0
#define GDRGPUObjectTransformPoolSlot 0
#define GDRGPUNodeTransformPoolSlot 1

struct GDRGPUGlobalData
{
	float4x4 VP; // camera view-proj
	float3 CameraPos; // Camera position
	float time; // Time in seconds
	
	float DeltaTime; // Delta time in seconds	
	UINT width;  // Screen size 
	UINT height; // Screen size 
	UINT pad[1];
};

struct GDRGPUObjectTransform
{
	float4x4 Transform; // Transform of Root of the object
	// AABB
	float3 minAABB;
	UINT pad[1];
	float3 maxAABB;
	UINT pad2[1];
};

#define NONE_INDEX 0xFFFFFFFF

struct GDRGPUNodeTransform
{
	float4x4 LocalTransform;  // Transform from parent
	float4x4 GlobalTransform; // Transform from ObjectTransform
	float4x4 BoneOffset;      // Transform for skinning
	UINT ParentIndex;         // Index of parent transform in pool. NONE_INDEX if no parent
	UINT ChildIndex;          // Index of first child in pool. NONE_INDEX if no child
	UINT NextIndex;           // Index of "Next" sibling in pool. NONE_INDEX if no next
	UINT IsNeedRecalc;        // Marks if we need this node to update
};

#ifdef __cplusplus
// C++ code
#pragma pack(pop)
#endif // __cplusplus
