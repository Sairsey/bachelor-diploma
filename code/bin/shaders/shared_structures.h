#pragma once

#ifdef __cplusplus
#include "utils/math/mth.h"
using float4x4 = mth::matr;
using float3 = mth::vec3f;
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
};

struct ObjectTransform
{
  float4x4 transform;
  float4x4 transformInversedTransposed;
  // AABB
  float3 minAABB;
  float3 maxAABB;
};

struct ObjectMaterial
{
  float3 Ka;
  float3 Kd;
  float3 Ks;
  float Ph;
  int KaMapIndex;
  int KdMapIndex;
  int KsMapIndex;
};

struct ObjectIndices
{
  UINT ObjectTransformIndex; // index of ObjectTransform
  UINT ObjectMaterialIndex; // index of ObjectMaterial
};

struct ComputeRootConstants
{
  float4x4 VP;
  float enableCulling;
  float commandCount;
};

#ifdef __cplusplus
// C++ code
#pragma pack(pop)
#endif // __cplusplus
