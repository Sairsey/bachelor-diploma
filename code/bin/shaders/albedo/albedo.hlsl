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
Texture2D TexturePool[] : register(GDRGPUTexturePoolSlot, GDRGPUTexturePoolSpace);                  // Bindless Pool with all textures
TextureCube CubeTexturePool[] : register(GDRGPUCubeTexturePoolSlot, GDRGPUCubeTexturePoolSpace); // Bindless Pool with all textures

SamplerState LinearSampler : register(GDRGPULinearSamplerSlot);  // Linear texture sampler
SamplerState NearestSampler : register(GDRGPUNearestSamplerSlot); // Nearest texture sampler

#include "shared/vertex_process.h"
#include "shared/lighting_functions.h"

VSOut VS(VSIn input)
{
	VSOut output = ProcessVSIn(input);
	return output;
}

[earlydepthstencil]
float4 PS(VSOut input) : SV_TARGET
{
	float3 normal = CalculateNormal(input.normal.xyz, input.tangent.xyz, input.uv, indices.ObjectMaterialIndex);

	float4 col = Shade(input.worldPos, normal, input.uv, indices.ObjectMaterialIndex);

	return col;
}
