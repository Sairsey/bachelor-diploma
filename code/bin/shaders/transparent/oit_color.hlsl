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
StructuredBuffer<GDRGPUBoneMapping> BoneMappingPool : register(GDRGPUBoneMappingSlot);                  // SRV: Data with bone mappings
Texture2D ShadowMapsPool[] : register(GDRGPUShadowMapsSRVSlot, GDRGPUShadowMapsSpace);                  // Bindless Pool with shadow maps
globallycoherent RWTexture2D<UINT> OITTexture : register(GDRGPUOITTextureUAVSlot);                      // UAV: Headers of lists
RWStructuredBuffer<GDRGPUOITNode> OITPool : register(GDRGPUOITPoolUAVSlot);                             // UAV: Elements of lists

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

[earlydepthstencil]
void PS(VSOut input, bool IsBackFace : SV_IsFrontFace)  // due to opengl matrices, CULL_MODE_FRONT and CULL_MODE_BACK are swapped
{
	if (indices.ObjectParamsMask & OBJECT_PARAMETER_FRONT_FACE_CULL && !IsBackFace)
		return;

	if (indices.ObjectParamsMask & OBJECT_PARAMETER_BACK_FACE_CULL && IsBackFace)
		return;

	float3 normal = CalculateNormal(input.normal.xyz, input.tangent.xyz, input.uv, indices.ObjectMaterialIndex);

	float depthSquared = dot(globals.CameraPos - input.worldPos.xyz, globals.CameraPos - input.worldPos.xyz);

	float val = dot(normal, globals.CameraPos - input.worldPos.xyz);
	normal *= val / abs(val);

	float4 col = Shade(input.worldPos, normal, input.uv, indices.ObjectMaterialIndex);
	uint newHeadBufferValue = OITPool.IncrementCounter();
	if (newHeadBufferValue >= globals.MaximumOITPoolSize) { return; };

	uint2 upos = uint2(input.pos.x - 0.5, input.pos.y - 0.5);
	uint OITTexW, OITTexH;
	OITTexture.GetDimensions(OITTexW, OITTexH);
	if (upos.x >= 0 && upos.x < OITTexW && upos.y >= 0 && upos.y < OITTexH)
	{
		uint previosHeadBufferValue;
		InterlockedExchange(OITTexture[upos], newHeadBufferValue, previosHeadBufferValue);

		// add element to the beginning of the list
		OITPool[newHeadBufferValue].NextNodeIndex = previosHeadBufferValue;
		OITPool[newHeadBufferValue].Depth = input.pos.z * (IsBackFace * 2 - 1);
		OITPool[newHeadBufferValue].Color = col;
	}

	return;
}
