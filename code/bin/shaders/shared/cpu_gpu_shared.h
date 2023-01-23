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
#define Space(x) x
#define SamplerSlot(x) x
#else
#define ConstantBufferSlot(x) b ## x
#define ShaderResourceSlot(x) t ## x
#define UnorderedAccessSlot(x) u ## x
#define Space(x) space ## x
#define SamplerSlot(x) s ## x
#endif // __cplusplus

// slots for all constant buffers
#define GDRGPUGlobalDataConstantBufferSlot ConstantBufferSlot(0)
#define GDRGPUObjectIndicesConstantBufferSlot ConstantBufferSlot(1)
#define GDRGPUComputeGlobalDataConstantBufferSlot ConstantBufferSlot(2)
// slots for all pools
#define GDRGPUObjectTransformPoolSlot ShaderResourceSlot(0)
#define GDRGPUNodeTransformPoolSlot ShaderResourceSlot(1)
#define GDRGPUMaterialPoolSlot ShaderResourceSlot(2)
#define GDRGPUTexturePoolSlot ShaderResourceSlot(3)
#define GDRGPUCubeTexturePoolSlot ShaderResourceSlot(4)
#define GDRGPUAllCommandsPoolSlot ShaderResourceSlot(5)
#define GDRGPUHierDepthSlot ShaderResourceSlot(6)
// slots for all uavs
#define GDRGPUOpaqueAllCommandsPoolSlot UnorderedAccessSlot(0)
#define GDRGPUTransparentAllCommandsPoolSlot UnorderedAccessSlot(1)
#define GDRGPUOpaqueFrustumCommandsPoolSlot UnorderedAccessSlot(2)
#define GDRGPUOpaqueCulledCommandsPoolSlot UnorderedAccessSlot(3)
#define GDRGPUTransparentsCulledCommandsPoolSlot UnorderedAccessSlot(4)

// Indexes of root params
#define GDRGPUObjectIndicesRecordRootIndex 1 // index in Root Signature for Record in ObjectIndices
// Spaces for bindless resources
#define GDRGPUTexturePoolSpace Space(1)
#define GDRGPUCubeTexturePoolSpace Space(2)
// Samplers Registers
#define GDRGPUNearestSamplerSlot SamplerSlot(0)
#define GDRGPULinearSamplerSlot SamplerSlot(1)


/// <summary>
///  Globals system
/// </summary>
struct GDRGPUComputeGlobals
{
	float4x4 VP;
	UINT frustumCulling;
	UINT occlusionCulling;
	UINT commandsCount;
	UINT width;  // Screen size 
	UINT height; // Screen size 
};

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
///  Materials system
/// </summary>

// improvised enum of types
#define MATERIAL_SHADER_COLOR 0                  // Diffuse component only
#define MATERIAL_SHADER_PHONG 1                  // Phong lighting model
#define MATERIAL_SHADER_COOKTORRANCE_METALNESS 2 // CookTorrance PBR model Metalness based
#define MATERIAL_SHADER_COOKTORRANCE_SPECULAR 3  // CookTorrance PBR model SPECULAR based
#define MATERIAL_SHADER_AMOUNT 4

struct GDRGPUMaterial
{
	UINT ShadeType;
	float3 VecParam0;

	UINT UintParam0;
	float3 VecParam1;

	UINT UintParam1;
	float3 VecParam2;

	UINT UintParam2;
	UINT UintParam3;
	float FloatParam0;
	float FloatParam1;
};
// define getters for this weired material structure

// SHADER_COLOR
#define GDRGPUMaterialColorGetColor(Material) Material.VecParam0
#define GDRGPUMaterialColorGetColorMapIndex(Material) Material.UintParam0
// SHADER_PHONG
#define GDRGPUMaterialPhongGetAmbient(Material) Material.VecParam0            // Ka
#define GDRGPUMaterialPhongGetAmbientMapIndex(Material) Material.UintParam1   // Ka map
#define GDRGPUMaterialPhongGetDiffuse(Material) Material.VecParam1            // Kd map
#define GDRGPUMaterialPhongGetDiffuseMapIndex(Material) Material.UintParam2   // Kd map
#define GDRGPUMaterialPhongGetSpecular(Material) Material.VecParam2           // Ks 
#define GDRGPUMaterialPhongGetSpecularMapIndex(Material) Material.UintParam3  // Ks map
#define GDRGPUMaterialPhongGetShiness(Material) Material.FloatParam0          // Ph
#define GDRGPUMaterialPhongGetNormalMapIndex(Material) Material.UintParam0    // Normal map
// SHADER_COOKTORRANCE_METALNESS
#define GDRGPUMaterialCookTorranceGetAlbedo(Material) Material.VecParam0                             // Albedo
#define GDRGPUMaterialCookTorranceGetAlbedoMapIndex(Material) Material.UintParam1                    // Albedo Map
#define GDRGPUMaterialCookTorranceGetRoughness(Material) Material.FloatParam0                        // roughness 
#define GDRGPUMaterialCookTorranceGetMetallic(Material) Material.FloatParam1                         // metallness
#define GDRGPUMaterialCookTorranceGetRoughnessMetallnessMapIndex(Material) Material.UintParam2       // roughness + metallness
#define GDRGPUMaterialCookTorranceGetNormalMapIndex(Material) Material.UintParam0                    // Normal map

#define ERROR_COLOR float4(1, 0, 1, 1);

// Some Object params
#define OBJECT_PARAMETER_TRANSPARENT 0x1

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
	UINT IsExist;                               // true if command is valid. False otherwise
	UINT _pad1[1];
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

#define GDRGPUComputeThreadBlockSize 1024

#ifdef __cplusplus
// C++ code
#pragma pack(pop)
#endif // __cplusplus
