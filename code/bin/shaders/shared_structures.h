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

struct GlobalData
{
  float4x4 VP; // camera view-proj
  float3 CameraPos; // Camera position
  float time; // Time in seconds
  UINT width;  // Screen size 
  UINT height; // Screen size 
  UINT LightsAmount; // Amount of active lights
};

struct ObjectTransform
{
  float4x4 transform;
  float4x4 transformInversedTransposed;
  // AABB
  float3 minAABB;
  float3 maxAABB;
};

struct LightSource
{
  UINT LightSourceType;      // type of lightsource to use
  UINT ObjectTransformIndex; // index of ObjectTransform
  float3 Color;              // Color of lightsource

  // Attenuation params
  float ConstantAttenuation;
  float LinearAttenuation;
  float QuadricAttenuation;

  // Spot light params
  float AngleInnerCone;              // for Spot - angle in radians
  float AngleOuterCone;              // for Spot - angle in radians
};

#define LIGHT_SOURCE_TYPE_DIRECTIONAL 0
#define LIGHT_SOURCE_TYPE_POINT 1
#define LIGHT_SOURCE_TYPE_SPOT 2

struct ObjectMaterial
{
  UINT ShadeType;
  float3 Ka;
  float3 Kd; // in case of PBR Kd.xyz - Albedo
  float3 Ks; // in case of PBR Ks.b - metallic, Ks.g - roughness
  float Ph;
  int KaMapIndex;
  int KdMapIndex;
  int KsMapIndex;
  int NormalMapIndex;
};

#define MATERIAL_SHADER_DIFFUSE 0
#define MATERIAL_SHADER_PHONG 1
#define MATERIAL_SHADER_COOKTORRANCE 2
#define MATERIAL_SHADER_AMOUNT 3

struct ObjectIndices
{
  UINT ObjectTransformIndex; // index of ObjectTransform
  UINT ObjectMaterialIndex; // index of ObjectMaterial
  UINT ObjectParams;        // MASK with some parameters for object
};

#define OBJECT_PARAMETER_TRANSPARENT 0x1       // Set if object is transparent

struct ComputeRootConstants
{
  float4x4 VP;
  UINT enableCulling;
  UINT commandCount;
};

struct OITList
{
  UINT RootIndex;
};

struct OITNode
{
  UINT NextNodeIndex;
  float Depth;
  float4 Color;
};

#define ComputeThreadBlockSize 1024
#define MAX_TRANSPARENT_ARRAY_SIZE 256
#define MAX_TRANSPARENT_DEPTH 10
#define MAX_AMOUNT_OF_TRANSPARENT_PIXELS (1920 * 1080 * MAX_TRANSPARENT_DEPTH)



#ifdef __cplusplus
// C++ code
#pragma pack(pop)
#endif // __cplusplus
