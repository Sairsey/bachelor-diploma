#pragma once
// This module depends on
// 
// ObjectTransformPool - which is a pool of GDRGPUObjectTransform
// MaterialPool - which is a pool of GDRGPUMaterial
// LightsPool - which is a pool of GDRGPULightSource
// GlobalValues - which is a constant buffer of GDRGPUGlobalData
// TexturePool - which is a bindless pool of 2d textures
// LinearSampler - which is a sampler with linear filtering
// NearestSampler - which is a sampler with linear filtering
#ifndef __cplusplus

bool CalcLight(in uint LightIndex, in float3 Position, out float3 DirectionToLight, out float3 LightColor)
{
  GDRGPULightSource light = LightsPool[LightIndex];

  if (light.ObjectTransformIndex == NONE_INDEX)
    return false;

  float attenuation = 1.0;
  if (light.LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
  {
    float3 LightDir = float3(0, -1, 0);
    float3x3 TransformMatrix = (float3x3)ObjectTransformPool[light.ObjectTransformIndex].Transform;
    LightDir = mul(TransformMatrix, LightDir).xyz;
    LightDir = normalize(LightDir);
    DirectionToLight = -LightDir;
  }
  else if (light.LightSourceType == LIGHT_SOURCE_TYPE_POINT)
  {
    float3 LightPos = float3(0, 0, 0);
    float4x4 TransformMatrix = ObjectTransformPool[light.ObjectTransformIndex].Transform;
    LightPos = mul(TransformMatrix, float4(LightPos, 1.0)).xyz;

    DirectionToLight = LightPos - Position;
    float d = length(DirectionToLight);
    DirectionToLight = normalize(DirectionToLight);

    attenuation = 1.0 / (
      light.ConstantAttenuation +
      light.LinearAttenuation * d +
      light.QuadricAttenuation * d * d);
  }
  else if (light.LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
  {
    float3 LightPos = float3(0, 0, 0);
    float4x4 TransformMatrix = ObjectTransformPool[light.ObjectTransformIndex].Transform;
    LightPos = mul(TransformMatrix, float4(LightPos, 1.0)).xyz;

    float3 LightDir = float3(0, -1, 0);
    TransformMatrix[0][3] = 0;
    TransformMatrix[1][3] = 0;
    TransformMatrix[2][3] = 0;
    LightDir = mul(TransformMatrix, float4(LightDir, 1.0)).xyz;
    LightDir = normalize(LightDir);

    DirectionToLight = LightPos - Position;
    float d = length(DirectionToLight);
    DirectionToLight = normalize(DirectionToLight);

    float cosTheta = dot(-DirectionToLight, LightDir);
    float cosInnerCone = cos(light.AngleInnerCone);
    float cosOuterCone = cos(light.AngleOuterCone);

    if (cosTheta < cosOuterCone)
    {
      return false;
    }

    attenuation = 1.0 / (
      light.ConstantAttenuation +
      light.LinearAttenuation * d +
      light.QuadricAttenuation * d * d);

    if (cosTheta < cosInnerCone)
      attenuation *= 1.0 - (cosInnerCone - cosTheta) / (cosInnerCone - cosOuterCone);
  }

  LightColor = attenuation * light.Color;
  return true;
}

float4 ShadeColor(float2 uv, GDRGPUMaterial material)
{
  float4 baseColor = float4(GDRGPUMaterialColorGetColor(material), 1);
  if (GDRGPUMaterialColorGetColorMapIndex(material) != NONE_INDEX)
    baseColor *= TexturePool[GDRGPUMaterialColorGetColorMapIndex(material)].Sample(LinearSampler, uv);
  return baseColor;
}

float4 ShadePhong(float3 Normal, float3 Position, float2 uv, GDRGPUMaterial material)
{
  float4 Ka = float4(GDRGPUMaterialPhongGetAmbient(material), 1);
  if (GDRGPUMaterialPhongGetAmbientMapIndex(material) != NONE_INDEX)
    Ka *= TexturePool[GDRGPUMaterialPhongGetAmbientMapIndex(material)].Sample(LinearSampler, uv);

  float4 Kd = float4(GDRGPUMaterialPhongGetDiffuse(material), 1);
  if (GDRGPUMaterialPhongGetDiffuseMapIndex(material) != NONE_INDEX)
    Kd *= TexturePool[GDRGPUMaterialPhongGetDiffuseMapIndex(material)].Sample(LinearSampler, uv);

  float4 Ks = float4(GDRGPUMaterialPhongGetSpecular(material), 1);
  if (GDRGPUMaterialPhongGetSpecularMapIndex(material) != NONE_INDEX)
    Ks *= TexturePool[GDRGPUMaterialPhongGetSpecularMapIndex(material)].Sample(LinearSampler, uv);

  float4 Ph = GDRGPUMaterialPhongGetShiness(material);
  float3 V = globals.CameraPos - Position;
  V = normalize(V);

  float4 resultColor = float4(0, 0, 0, Kd.a);

  for (uint i = 0; i < globals.LightsAmount; i++)
  {
    float3 L;
    float3 LColor;

    if (!CalcLight(i, Position, L, LColor))
      continue;

    float NdotL = dot(Normal, L);

    if (NdotL > 0)
    {
      float3 R = reflect(-L, Normal);

      resultColor.xyz += LColor * Kd.xyz * NdotL;

      float RdotV = dot(R, V);
      resultColor.xyz += LColor * Ks.xyz * pow(max(0.0f, RdotV), Ph);
    }
  }

  return resultColor;
}

float4 Shade(float3 pos, float3 norm, float2 uv, uint materialIndex)
{
    if (materialIndex == NONE_INDEX)
        return ERROR_COLOR;

    GDRGPUMaterial material = MaterialPool[materialIndex];
    switch (material.ShadeType)
    {
    case MATERIAL_SHADER_COLOR:
      return ShadeColor(uv, material);
    case MATERIAL_SHADER_PHONG:
      return ShadePhong(norm, pos, uv, material);
    default:
      return ERROR_COLOR;
  }
}

float3 CalculateNormal(float3 norm, float3 tangent, float2 uv, uint materialIndex)
{
  GDRGPUMaterial material = MaterialPool[materialIndex];
  float3 Normal = normalize(norm);
  if (material.ShadeType == MATERIAL_SHADER_PHONG)
  {
    float3 Tangent = normalize(tangent);
    float3 Bitangent = cross(Normal, Tangent);
    float3x3 TBN = transpose(float3x3(Tangent, Bitangent, Normal));

    if (GDRGPUMaterialPhongGetNormalMapIndex(material) != NONE_INDEX)
    {
      Normal = TexturePool[GDRGPUMaterialPhongGetNormalMapIndex(material)].Sample(LinearSampler, uv).xyz;
      Normal = (Normal * 2.0 - 1.0);
      Normal = normalize(mul(TBN, Normal));
    }
  }
  return Normal;
}

#endif // __cplusplus