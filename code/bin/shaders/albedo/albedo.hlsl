#include "shared/cpu_gpu_shared.h"
#include "shared/lighting_functions.h"

cbuffer GlobalValues : register (GDRGPUGlobalDataConstantBufferSlot)
{
	GDRGPUGlobalData globals;
}

cbuffer Indices : register(GDRGPUObjectIndicesConstantBufferSlot)
{
	GDRGPUObjectIndices indices;
}
StructuredBuffer<GDRGPUObjectTransform> ObjectTransformPool : register(GDRGPUObjectTransformPoolSlot);    // SRV: Data with transforms which stored per object
StructuredBuffer<GDRGPUNodeTransform> NodeTransformPool : register(GDRGPUNodeTransformPoolSlot);        // SRV: Data with transforms which stored for whole hierarchy

#include "shared/vertex_process.h"

VSOut VS(VSIn input)
{
	VSOut output = ProcessVSIn(input);
	return output;
}

[earlydepthstencil]
float4 PS(VSOut input) : SV_TARGET
{
	float4 col = Shade();
	return col;
}
