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
#define GDRGPUGlobalDataConstantBufferSlot ConstantBufferSlot(1)
#define GDRGPUObjectIndicesConstantBufferSlot ConstantBufferSlot(2)
#define GDRGPUComputeGlobalDataConstantBufferSlot ConstantBufferSlot(3)
#define GDRGPULuminanceConstantBufferSlot ConstantBufferSlot(4)
#define GDRGPUEnviromentConstantBufferSlot ConstantBufferSlot(5)
// slots for all pools
#define GDRGPUObjectTransformPoolSlot ShaderResourceSlot(1)
#define GDRGPUNodeTransformPoolSlot ShaderResourceSlot(2)
#define GDRGPUMaterialPoolSlot ShaderResourceSlot(3)
#define GDRGPUTexturePoolSlot ShaderResourceSlot(4)
#define GDRGPUCubeTexturePoolSlot ShaderResourceSlot(5)
#define GDRGPULightsPoolSlot ShaderResourceSlot(6)
#define GDRGPUAllCommandsPoolSlot ShaderResourceSlot(7)
#define GDRGPUHierDepthSlot ShaderResourceSlot(8)
#define GDRGPUBoneMappingSlot ShaderResourceSlot(9)
#define GDRGPUOITTextureSRVSlot ShaderResourceSlot(10)
#define GDRGPUOITPoolSRVSlot ShaderResourceSlot(11)
// slots for all uavs
#define GDRGPUOpaqueAllCommandsPoolSlot UnorderedAccessSlot(1)
#define GDRGPUTransparentAllCommandsPoolSlot UnorderedAccessSlot(2)
#define GDRGPUOpaqueFrustumCommandsPoolSlot UnorderedAccessSlot(3)
#define GDRGPUOpaqueCulledCommandsPoolSlot UnorderedAccessSlot(4)
#define GDRGPUTransparentsCulledCommandsPoolSlot UnorderedAccessSlot(5)
#define GDRGPULuminanceUnorderedAccessBufferSlot UnorderedAccessSlot(6)
#define GDRGPUOITTextureUAVSlot UnorderedAccessSlot(7)
#define GDRGPUOITPoolUAVSlot UnorderedAccessSlot(8)

// Parameters for any shit we might need, but do not want to map
#define GDRGPUUserConstantBuffer1Slot ConstantBufferSlot(20)
#define GDRGPUUserConstantBuffer2Slot ConstantBufferSlot(21)
#define GDRGPUUserConstantBuffer3Slot ConstantBufferSlot(22)
#define GDRGPUUserConstantBuffer4Slot ConstantBufferSlot(23)

#define GDRGPUUserShaderResource1Slot ShaderResourceSlot(20)
#define GDRGPUUserShaderResource2Slot ShaderResourceSlot(21)
#define GDRGPUUserShaderResource3Slot ShaderResourceSlot(22)
#define GDRGPUUserShaderResource4Slot ShaderResourceSlot(23)

#define GDRGPUUserUnorderedAccess1Slot UnorderedAccessSlot(20)
#define GDRGPUUserUnorderedAccess2Slot UnorderedAccessSlot(21)
#define GDRGPUUserUnorderedAccess3Slot UnorderedAccessSlot(22)
#define GDRGPUUserUnorderedAccess4Slot UnorderedAccessSlot(23)


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

/// Enviroment system
struct GDRGPUEnviromentData
{
	UINT SkyboxIndex;             // Index of skybox in cube textures pool
	UINT PrefilteredCubemapIndex; // Index of prefiltered color texture in cube textures pool
	UINT BRDFLUTIndex;            // Index of brdf look-up table
	UINT IrradianceCubemapIndex;  // Index of irradiance texture in cube textures pool
	UINT MaxReflectionLod;        // Max reflection LOD
	UINT pad[3];
};

/// <summary>
///  Globals system
/// </summary>
struct GDRGPUGlobalData
{
	float4x4 VP; // camera view-proj

	float3 CameraPos; // Camera position
	float Time; // Time in seconds
	
	float DeltaTime; // Delta time in seconds	
	UINT Width;  // Screen size 
	UINT Height; // Screen size 
	UINT LightsAmount; // Amount of lights in scene

	UINT IsTonemap;      // Index of skybox in cube textures pool
	float SceneExposure; // Additional exposure from scene
	UINT IsIBL;          // Enable IBL
	UINT MaximumOITPoolSize; // Maximum size of OIT Pool

	UINT DebugOIT;           // True if we want to debug OIT
	UINT IsFXAA;             // True if we want to use FXAA
	UINT pad[2];
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
/// Bone mapping system
/// </summary>
#define MAX_BONE_PER_MESH 512
struct GDRGPUBoneMapping
{
	UINT BoneMapping[MAX_BONE_PER_MESH];
};

/// <summary>
/// Node Transform system
/// </summary>
#ifndef NONE_INDEX
#define NONE_INDEX 0xFFFFFFFF
#endif // !NONE_INDEX
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
///  Lights system
/// </summary>

// improvised enum of types
#define LIGHT_SOURCE_TYPE_DIRECTIONAL 0
#define LIGHT_SOURCE_TYPE_POINT 1
#define LIGHT_SOURCE_TYPE_SPOT 2
#define LIGHT_SOURCE_TYPE_AMOUNT 3

struct GDRGPULightSource
{
	UINT LightSourceType;      // type of lightsource to use
	UINT ObjectTransformIndex; // index of ObjectTransform
	UINT ShadowMapIndex;       // NONE if no shadow. Valid number overwise
	// Attenuation params
	float ConstantAttenuation;
	
	float LinearAttenuation;
	float QuadricAttenuation;
	// Spot light params
	float AngleInnerCone;              // for Spot - angle in radians
	float AngleOuterCone;              // for Spot - angle in radians
	
	float4x4 VP;               // ViewProj matrix of light source
	float4x4 InvVP;            // InverseViewProj matrix of light source
	float3 Color;              // Color of lightsource
	UINT pad[1];
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

	float FloatParam2;
	UINT pad[3];
};
// define getters for this weired material structure

// SHADER_COLOR
#define GDRGPUMaterialColorGetColor(Material) Material.VecParam0
#define GDRGPUMaterialColorGetColorMapIndex(Material) Material.UintParam0
#define GDRGPUMaterialColorGetOpacity(Material) Material.FloatParam2
// SHADER_PHONG
#define GDRGPUMaterialPhongGetAmbient(Material) Material.VecParam0            // Ka
#define GDRGPUMaterialPhongGetAmbientMapIndex(Material) Material.UintParam1   // Ka map
#define GDRGPUMaterialPhongGetOpacity(Material) Material.FloatParam2          // Opacity
#define GDRGPUMaterialPhongGetDiffuse(Material) Material.VecParam1            // Kd map
#define GDRGPUMaterialPhongGetDiffuseMapIndex(Material) Material.UintParam2   // Kd map
#define GDRGPUMaterialPhongGetSpecular(Material) Material.VecParam2           // Ks 
#define GDRGPUMaterialPhongGetSpecularMapIndex(Material) Material.UintParam3  // Ks map
#define GDRGPUMaterialPhongGetShiness(Material) Material.FloatParam0          // Ph
#define GDRGPUMaterialPhongGetNormalMapIndex(Material) Material.UintParam0    // Normal map
// SHADER_COOKTORRANCE_METALNESS
// contain 5 variables
// 1) Ambient Occlusion
// 2) Albedo
// 3) Metallness
// 4) Roughness
// 5) Normal Map
#define GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(Material) Material.UintParam1          // Ambient Occlusion Map
#define GDRGPUMaterialCookTorranceGetAlbedo(Material) Material.VecParam0                             // Albedo
#define GDRGPUMaterialCookTorranceGetAlbedoMapIndex(Material) Material.UintParam2                    // Albedo Map
#define GDRGPUMaterialCookTorranceGetRoughness(Material) Material.FloatParam0                        // Roughness 
#define GDRGPUMaterialCookTorranceGetMetalness(Material) Material.FloatParam1                        // metalness
#define GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(Material) Material.UintParam3        // roughness(g) + metallness(b)
#define GDRGPUMaterialCookTorranceGetNormalMapIndex(Material) Material.UintParam0                    // Normal map
#define GDRGPUMaterialCookTorranceGetOpacity(Material) Material.FloatParam2                          // Opacity
// MATERIAL_SHADER_COOKTORRANCE_SPECULAR
// contain 5 variables
// 1) Ambient Occlusion (Same)
// 2) Albedo            (Same)
// 3) Specular          
// 4) Glossiness        (1.0 - Roughness)
// 5) Normal Map        (Same)
#define GDRGPUMaterialCookTorranceGetSpecular(Material) Material.VecParam1                            // Specular
#define GDRGPUMaterialCookTorranceGetGlossiness(Material) Material.FloatParam0                        // Glossiness
#define GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(Material) Material.UintParam3         // Specular(rgb) + Glossiness(a)
#define GDRGPUMaterialCookTorranceGetOpacity(Material) Material.FloatParam2                           // Opacity

#define ERROR_COLOR float4(1, 0, 1, 1);

// Some Object params
#define OBJECT_PARAMETER_TRANSPARENT 0x1
#define OBJECT_PARAMETER_FRONT_FACE_CULL 0x2
#define OBJECT_PARAMETER_BACK_FACE_CULL 0x4

/// <summary>
/// Object system
/// </summary>
struct GDRGPUObjectIndices
{
	UINT ObjectIndex;          // On CPU -> geometry index. On GPU -> filled on GPU with index 
	UINT ObjectParamsMask;     // Flags of current object
	UINT ObjectTransformIndex; // index in Object Transforms pool
	UINT ObjectMaterialIndex;  // index in Object Materials pool
	UINT BoneMappingIndex;     // index in Bone Mapping pool
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
	//UINT _pad1[0];
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

/// <summary>
/// Luminance system
/// </summary>
struct GDRGPULuminanceVariables
{
	float Luminance;
	float LuminanceAdapted;
	float Exposure;
	float ExposureAdapted;
};

#define GDRGPUComputeThreadBlockSize 1024

/// <summary>
/// Order Independent transparency system
/// </summary>
struct GDRGPUOITNode
{
	float4 Color;
	float Depth;
	UINT NextNodeIndex;
};

#ifdef __cplusplus
// C++ code
#pragma pack(pop)
#endif // __cplusplus

// add command to sample from cube texture on GPU
#ifndef __cplusplus
// in project I use right-handed CS. But HLSL uses left-handed CS.
// So to fix it I need my special sampler function, which will reverse Z for me
float4 GDRSampleCube(TextureCube cubeTexture, SamplerState mySampler, float3 dir)
{
	dir.z = -dir.z;
	return cubeTexture.Sample(mySampler, dir);
}

// in project I use right-handed CS. But HLSL uses left-handed CS.
// So to fix it I need my special sampler function, which will reverse Z for me
float4 GDRSampleCubeLevel(TextureCube cubeTexture, SamplerState mySampler, float3 dir, float level)
{
	dir.z = -dir.z;
	return cubeTexture.SampleLevel(mySampler, dir, level);
}

#endif // __cplusplus
