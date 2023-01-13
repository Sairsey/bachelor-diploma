#pragma once
// This module depends on
// MaterialPool which is a pool of GDRGPUMaterial
// globals which is a constant buffer of GDRGPUGlobalData
#ifndef __cplusplus
float4 Shade(float3 pos, float3 norm, uint materialIndex)
{
    if (materialIndex == NONE_INDEX)
        return float4(1, 0, 1, 1);

    GDRGPUMaterial material = MaterialPool[materialIndex];
    switch (material.ShadeType)
    {
    case MATERIAL_SHADER_COLOR:
        return float4(GDRGPUMaterialColorGetColor(material), 1);
    default:
        return float4(1, 0, 1, 1);
  }
}

#endif // __cplusplus