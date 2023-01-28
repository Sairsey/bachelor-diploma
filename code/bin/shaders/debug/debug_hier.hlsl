#include "shared/cpu_gpu_shared.h"

cbuffer GlobalParams : register(GDRGPUUserConstantBuffer1Slot)
{
	float4x4 VP;
	uint index;
};

StructuredBuffer<GDRGPUNodeTransform> NodeTransformPool : register(GDRGPUNodeTransformPoolSlot);        // SRV: Data with transforms which stored for whole hierarchy

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
	float4x4 matr;
	if (input.pos.x == 1)
		matr = NodeTransformPool[index].GlobalTransform;
	else
		matr = NodeTransformPool[NodeTransformPool[index].ParentIndex].GlobalTransform;
	output.pos = float4(matr[0][3], matr[1][3], matr[2][3], 1);
	output.pos = mul(VP, output.pos);
	return output;
}

[earlydepthstencil]
float4 PS(VSOut input) : SV_TARGET
{
	return float4(1, 0, 0, 1);
}