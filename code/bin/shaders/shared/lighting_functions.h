#pragma once
// This module depends on
// 
// MaterialPool - which is a pool of GDRGPUMaterial
// GlobalValues - which is a constant buffer of GDRGPUGlobalData
// TexturePool - which is a bindless pool of 2d textures
// LinearSampler - which is a sampler with linear filtering
// NearestSampler - which is a sampler with linear filtering
#ifndef __cplusplus

float4 ShadeColor(float2 uv, GDRGPUMaterial material)
{
  float4 baseColor = float4(GDRGPUMaterialColorGetColor(material), 1);
  if (GDRGPUMaterialColorGetColorMapIndex(material) != NONE_INDEX)
    baseColor *= TexturePool[GDRGPUMaterialColorGetColorMapIndex(material)].Sample(LinearSampler, uv);
  return baseColor;
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
    default:
        return ERROR_COLOR;
  }
}

#endif // __cplusplus