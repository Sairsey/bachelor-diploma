#include "shared/cpu_gpu_shared.h"

cbuffer GlobalValues : register (GDRGPUGlobalDataConstantBufferSlot)
{
	GDRGPUGlobalData globals;
}
cbuffer EnviromentValues : register (GDRGPUEnviromentConstantBufferSlot)
{
	GDRGPUEnviromentData env;
}
TextureCube CubeTexturePool[] : register(GDRGPUCubeTexturePoolSlot, GDRGPUCubeTexturePoolSpace); // Bindless Pool with all textures

SamplerState LinearSampler : register(GDRGPULinearSamplerSlot);  // Linear texture sampler
SamplerState NearestSampler : register(GDRGPUNearestSamplerSlot); // Nearest texture sampler

struct VSIn
{
	float3 pos : POSITION;
};

struct VSOut
{
	float4 pos : SV_POSITION;
	float3 TexCoords : TEXCOORDS;
};

VSOut VS(VSIn input)
{
	VSOut output;
	float4x4 transform = globals.VP;
	output.pos = mul(transform, float4(input.pos + globals.CameraPos, 1)).xyww;
	output.TexCoords = normalize(input.pos);
	return output;
}

[earlydepthstencil]
float4 PS(VSOut input) : SV_TARGET
{
	if (env.SkyboxIndex == NONE_INDEX)
		return float4(0., 0., 0.0, 1);
	else
		return GDRSampleCube(CubeTexturePool[env.SkyboxIndex], LinearSampler, input.TexCoords).rgba;
}
