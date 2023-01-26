#pragma once
// This module depends on
// 
// ObjectTransformPool - which is a pool of GDRGPUObjectTransform
// MaterialPool - which is a pool of GDRGPUMaterial
// LightsPool - which is a pool of GDRGPULightSource
// GlobalValues - which is a constant buffer of GDRGPUGlobalData
// EnviromentValues - which is a constant buffer of GDRGPUEnviromentData
// TexturePool - which is a bindless pool of 2d textures
// CubeTexturePool - which is a bindless pool of cube textures
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

/// <summary>
/// SHADER_COLOR
/// </summary>
float4 ShadeColor(float2 uv, GDRGPUMaterial material)
{
  float4 baseColor = float4(GDRGPUMaterialColorGetColor(material), 1);
  if (GDRGPUMaterialColorGetColorMapIndex(material) != NONE_INDEX)
    baseColor *= TexturePool[GDRGPUMaterialColorGetColorMapIndex(material)].Sample(LinearSampler, uv);
  return baseColor;
}

/// <summary>
/// SHADER_PHONG
/// </summary>
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
      resultColor.xyz += LColor * Ks.xyz * (pow(max(0.0f, RdotV), Ph)).xxx;
    }
  }

  return resultColor;
}

struct PBRParams
{
    float AmbientOcclusion;
    float Transparency;
    // Taken from https://github.com/KhronosGroup/glTF/issues/810
    float3 Diffuse;
    float3 Specular;
    float Roughness;
};

#define PI 3.14159265358979323846

float GGX_PartialGeometry(float cosThetaN, float alpha) {
    float K = (alpha + 1) * (alpha + 1) / 8.0;
    return cosThetaN / (cosThetaN * (1 - K) + K);
}

float GGX_Distribution(float cosThetaNH, float alpha) {
    float alpha2 = alpha * alpha;
    float NH2 = cosThetaNH * cosThetaNH;
    float den = NH2 * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * den * den);
}

float3 FresnelSchlick(float3 F0, float cosTheta) {
    return F0 + ((1.0).xxx - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickEnv(float3 F0, float cosTheta, float alpha) {
    return F0 + (max((1.0 - alpha).xxx, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float4 ShadeCookTorrance(float3 Normal, float3 Position, float2 uv, GDRGPUMaterial material, PBRParams Params)
{
    float3 resultColor = float3(0, 0, 0);

    float3 V = globals.CameraPos - Position;
    V = normalize(V);

    float3 F0 = Params.Specular;
    float NV = max(dot(Normal, V), 0.0);
    for (uint i = 0; i < globals.LightsAmount; i++)
    {
        float3 L;
        float3 LColor;

        if (!CalcLight(i, Position, L, LColor))
            continue;

        float3 H = normalize(V + L);
        //precompute dots
        float NL = max(dot(Normal, L), 0.0);
        float NH = max(dot(Normal, H), 0.0);
        float HV = max(dot(H, V), 0.0);

        //precompute roughness square
        float G = GGX_PartialGeometry(NV, Params.Roughness) * GGX_PartialGeometry(NL, Params.Roughness);
        float D = GGX_Distribution(NH, Params.Roughness);

        float3 F = FresnelSchlick(F0, HV);
        float3 KSpecular = F;
        float3 Specular = KSpecular * G * D / max(4.0 * (NV) * (NL), 0.001);
        float3 KDiffuse = float3(1.0, 1.0, 1.0) - KSpecular;
        float3 Diffuse = Params.Diffuse / PI * KDiffuse;

        resultColor += LColor * (Diffuse + Specular) * NL;
    }

    if (globals.IsIBL && env.PrefilteredCubemapIndex != NONE_INDEX)
    {
        float3 r = reflect(-V, Normal);

        float3 F = FresnelSchlickEnv(F0, NV, Params.Roughness);
        float3 KSpecular = F;
        float3 KDiffuse = float3(1.0, 1.0, 1.0) - F;

        static const float MAX_REFLECTION_LOD = 4.0;
        float3 prefilteredColor = CubeTexturePool[env.PrefilteredCubemapIndex].SampleLevel(LinearSampler, float3(r.xy, -r.z), Params.Roughness * MAX_REFLECTION_LOD).rgb;
        float2 envBRDF = TexturePool[env.BRDFLUTIndex].Sample(LinearSampler, float2(NV, Params.Roughness)).rg;
        float3 Specular = prefilteredColor * (F0 * envBRDF.x + envBRDF.y);

        float3 Irradience = CubeTexturePool[env.IrradianceCubemapIndex].Sample(LinearSampler, float3(Normal.xy, -Normal.z)).rgb;
        float3 Diffuse = Irradience * Params.Diffuse;

        float3 Ambient = KDiffuse * Diffuse + Specular;
        resultColor += Ambient;

    }

    return float4(resultColor * Params.AmbientOcclusion, Params.Transparency);
}


/// <summary>
/// SHADER_COOKTORRANCE_METALNESS
/// </summary>
float4 ShadeCookTorranceMetal(float3 Normal, float3 Position, float2 uv, GDRGPUMaterial material)
{
    PBRParams Params;

    // Ambient occlusion
    Params.AmbientOcclusion = 1.0;
    if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(material) != NONE_INDEX)
        Params.AmbientOcclusion = TexturePool[GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(material)].Sample(LinearSampler, uv).x;
    
    // Roughness + Metalness
    Params.Roughness = GDRGPUMaterialCookTorranceGetRoughness(material);
    float Metalness = GDRGPUMaterialCookTorranceGetMetalness(material);
    if (GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(material) != NONE_INDEX)
    {
        float4 tmp = TexturePool[GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(material)].Sample(LinearSampler, uv);
        Params.Roughness *= tmp.g;
        Metalness *= tmp.b;
    }

    // Diffuse + Transparency
    Params.Diffuse = GDRGPUMaterialCookTorranceGetAlbedo(material);
    Params.Transparency = 1.0;
    if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(material) != NONE_INDEX)
    {
        float4 tmp = TexturePool[GDRGPUMaterialCookTorranceGetAlbedoMapIndex(material)].Sample(LinearSampler, uv);
        Params.Diffuse = tmp.rgb;
        Params.Transparency = tmp.a;
    }

    // Specular
    Params.Specular = 0.04;
    Params.Specular = lerp(Params.Specular, Params.Diffuse, Metalness);

    Params.Diffuse *= (1 - Metalness);
    Params.Roughness = max(Params.Roughness, 0.001);

    return ShadeCookTorrance(Normal, Position, uv, material, Params);
}

/// <summary>
/// SHADER_COOKTORRANCE_SPECULAR
/// </summary>
float4 ShadeCookTorranceSpecular(float3 Normal, float3 Position, float2 uv, GDRGPUMaterial material)
{
    PBRParams Params;

    // Ambient occlusion
    Params.AmbientOcclusion = 1.0;
    if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(material) != NONE_INDEX)
        Params.AmbientOcclusion = TexturePool[GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(material)].Sample(LinearSampler, uv).xyz;

    // Glossiness + Specular (and Roughness too)
    Params.Roughness = GDRGPUMaterialCookTorranceGetGlossiness(material);
    Params.Specular = GDRGPUMaterialCookTorranceGetSpecular(material);
    if (GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(material) != NONE_INDEX)
    {
        float4 tmp = TexturePool[GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(material)].Sample(LinearSampler, uv);
        Params.Roughness *= tmp.a;
        Params.Specular *= tmp.rgb;
    }
    Params.Roughness = 1 - Params.Roughness;

    // Diffuse + Transparency
    Params.Diffuse = GDRGPUMaterialCookTorranceGetAlbedo(material);
    Params.Transparency = 1.0;
    if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(material) != NONE_INDEX)
    {
        float4 tmp = TexturePool[GDRGPUMaterialCookTorranceGetAlbedoMapIndex(material)].Sample(LinearSampler, uv);
        Params.Diffuse *= tmp.rgb;
        Params.Transparency = tmp.a;
    }
    Params.Roughness = max(Params.Roughness, 0.001);
    return ShadeCookTorrance(Normal, Position, uv, material, Params);
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
    case MATERIAL_SHADER_COOKTORRANCE_METALNESS:
        return ShadeCookTorranceMetal(norm, pos, uv, material);
    case MATERIAL_SHADER_COOKTORRANCE_SPECULAR:
        return ShadeCookTorranceSpecular(norm, pos, uv, material);
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
  else if (material.ShadeType == MATERIAL_SHADER_COOKTORRANCE_METALNESS || material.ShadeType == MATERIAL_SHADER_COOKTORRANCE_SPECULAR)
  {
    float3 Tangent = normalize(tangent);
    float3 Bitangent = cross(Normal, Tangent);
    float3x3 TBN = transpose(float3x3(Tangent, Bitangent, Normal));

    if (GDRGPUMaterialCookTorranceGetNormalMapIndex(material) != NONE_INDEX)
    {
        Normal = TexturePool[GDRGPUMaterialCookTorranceGetNormalMapIndex(material)].Sample(LinearSampler, uv).xyz;
        Normal = (Normal * 2.0 - 1.0);
        Normal = normalize(mul(TBN, Normal));
    }
  }
  return Normal;
}

#endif // __cplusplus