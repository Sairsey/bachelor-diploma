#include "shared/cpu_gpu_shared.h"

cbuffer GlobalValues : register (GDRGPUGlobalDataConstantBufferSlot)
{
	GDRGPUGlobalData globals;
}
cbuffer EnviromentValues : register (GDRGPUEnviromentConstantBufferSlot)
{
	GDRGPUEnviromentData env;
}
cbuffer Indices : register(GDRGPUObjectIndicesConstantBufferSlot)
{
	GDRGPUObjectIndices indices;
}
StructuredBuffer<GDRGPUObjectTransform> ObjectTransformPool : register(GDRGPUObjectTransformPoolSlot);  // SRV: Data with transforms which stored per object
StructuredBuffer<GDRGPUNodeTransform> NodeTransformPool : register(GDRGPUNodeTransformPoolSlot);        // SRV: Data with transforms which stored for whole hierarchy
StructuredBuffer<GDRGPUMaterial> MaterialPool : register(GDRGPUMaterialPoolSlot);                       // SRV: Data with materials
StructuredBuffer<GDRGPULightSource> LightsPool : register(GDRGPULightsPoolSlot);                        // SRV: Data with light sources
Texture2D TexturePool[] : register(GDRGPUTexturePoolSlot, GDRGPUTexturePoolSpace);                      // Bindless Pool with all textures
TextureCube CubeTexturePool[] : register(GDRGPUCubeTexturePoolSlot, GDRGPUCubeTexturePoolSpace);        // Bindless Pool with all textures
Texture2D ShadowMapsPool[] : register(GDRGPUShadowMapsSRVSlot, GDRGPUShadowMapsSpace);                  // Bindless Pool with shadow maps
StructuredBuffer<GDRGPUBoneMapping> BoneMappingPool : register(GDRGPUBoneMappingSlot);                  // SRV: Data with bone mappings

SamplerState LinearSampler : register(GDRGPULinearSamplerSlot);  // Linear texture sampler
SamplerState NearestSampler : register(GDRGPUNearestSamplerSlot); // Nearest texture sampler
SamplerComparisonState ShadowSampler : register(GDRGPUShadowSamplerSlot); // Shadow texture sampler

#include "shared/vertex_process.h"
#include "shared/lighting_functions.h"

VSOut VS(VSIn input)
{
	VSOut output = ProcessVSIn(input);
	return output;
}


float4 PS(VSOut input, bool IsFrontFace : SV_IsFrontFace) : SV_TARGET
{
	if (indices.ObjectParamsMask & OBJECT_PARAMETER_FRONT_FACE_CULL && IsFrontFace)
		discard;

	if (indices.ObjectParamsMask & OBJECT_PARAMETER_BACK_FACE_CULL && !IsFrontFace)
		discard;
	
	float3 normal = CalculateNormal(input.normal.xyz, input.tangent.xyz, input.uv, indices.ObjectMaterialIndex);

	float4 col = Shade(input.worldPos, normal, input.uv, indices.ObjectMaterialIndex);
	return col;
}
