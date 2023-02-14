#include "shared/cpu_gpu_shared.h"

cbuffer GlobalParams : register(GDRGPUUserConstantBuffer1Slot)
{
	float4x4 VP;
	float4x4 transform;
	float3 minAABB;
	float pad1;
	float3 maxAABB;
	float pad2;
	float3 color;
	float pad3;
};

struct VSIn
{
	float3 pos : POSITION;
};

struct VSOut
{
	float4 pos : SV_POSITION;
};

VSOut VS(VSIn input)
{
	VSOut output;
	output.pos = float4(input.pos, 1);
	output.pos.x = output.pos.x * (maxAABB.x - minAABB.x) / 2.0 + (maxAABB.x + minAABB.x) / 2.0;
	output.pos.y = output.pos.y * (maxAABB.y - minAABB.y) / 2.0 + (maxAABB.y + minAABB.y) / 2.0;
	output.pos.z = output.pos.z * (maxAABB.z - minAABB.z) / 2.0 + (maxAABB.z + minAABB.z) / 2.0;
	output.pos = mul(transform, output.pos);
	output.pos = mul(VP, output.pos);
	return output;
}

[earlydepthstencil]
float4 PS(VSOut input) : SV_TARGET
{
	return float4(color, 1);
}