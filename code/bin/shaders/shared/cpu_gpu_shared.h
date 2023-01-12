#pragma once

#ifdef __cplusplus
#include "utils/math/mth.h"
using float4x4 = mth::matr;
using float3 = mth::vec3f;
using float4 = mth::vec4f;
using float2 = mth::vec2f;
using int2 = mth::vec2<int>;
using int4 = mth::vec4<int>;
using uint2 = mth::vec2<UINT>;
using uint4 = mth::vec4<UINT>;
// C++ code
#pragma pack(push, 1)
#else
// HLSL code
#define UINT uint
#endif // __cplusplus

#ifdef __cplusplus
#include "indirect_structures.h"
#else
#include "shared/indirect_structures.h"
#endif

#ifdef __cplusplus
#define ConstantBufferSlot(x) x
#define ShaderResourceSlot(x) x
#define UnorderedAccessSlot(x) x
#else
#define ConstantBufferSlot(x) b ## x
#define ShaderResourceSlot(x) t ## x
#define UnorderedAccessSlot(x) u ## x

#endif // __cplusplus

// slots for all constant buffers
#define GDRGPUGlobalDataConstantBufferSlot ConstantBufferSlot(0)
#define GDRGPUObjectIndicesConstantBufferSlot ConstantBufferSlot(1)
// slots for all pools
#define GDRGPUObjectTransformPoolSlot ShaderResourceSlot(0)
#define GDRGPUNodeTransformPoolSlot ShaderResourceSlot(1)

// Indexes of root params
#define GDRGPUObjectIndicesRecordRootIndex 1 // index in Root Signature for Record in ObjectIndices

/// <summary>
///  Globals system
/// </summary>
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

/// <summary>
/// Object Transform system
/// </summary>
struct GDRGPUObjectTransform
{
	float4x4 Transform; // Transform of Root of the object
	// AABB
	float3 minAABB;
	UINT pad[1];
	float3 maxAABB;
	UINT pad2[1];
};


/// <summary>
/// Node Transform system
/// </summary>
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

/// <summary>
/// Object system
/// </summary>
struct GDRGPUObjectIndices
{
	UINT ObjectIndex;          // Filled on GPU
	UINT ObjectParamsMask;     // Flags of current object
	UINT ObjectTransformIndex; // index in Object Transforms pool
	UINT ObjectMaterialIndex;  // index in Object Materials pool
};

/// <summary>
/// Indirect system
/// </summary>
struct GDRGPUIndirectCommand
{
	GDRGPUObjectIndices Indices;                // we want to set buffer with indices
	D3D12_VERTEX_BUFFER_VIEW VertexBuffer;      // set correct vertex buffer
	D3D12_INDEX_BUFFER_VIEW IndexBuffer;        // set correct index buffer
	D3D12_DRAW_INDEXED_ARGUMENTS DrawArguments; // then draw indirect indexed primitive
	UINT _pad1[2];
};

/// <summary>
/// Geometry system
/// </summary>
struct GDRVertex
{
	float3 Pos;
	float3 Normal;
	float2 UV;
	float3 Tangent;
	uint4 BonesIndices;
	float4 BonesWeights;
};

#ifdef __cplusplus
// C++ code
#pragma pack(pop)
#endif // __cplusplus
