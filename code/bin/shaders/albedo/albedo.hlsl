#include "shared/cpu_gpu_shared.h"

cbuffer GlobalValues : register (GDRGPUGlobalDataConstantBufferSlot)
{
	GDRGPUGlobalData globals;
}

cbuffer Indices : register(GDRGPUObjectIndicesConstantBufferSlot)
{
	GDRGPUObjectIndices indices;
}
StructuredBuffer<GDRGPUObjectTransform> ObjectTransformPool : register(GDRGPUObjectTransformPoolSlot);  // SRV: Data with transforms which stored per object
StructuredBuffer<GDRGPUNodeTransform> NodeTransformPool : register(GDRGPUNodeTransformPoolSlot);        // SRV: Data with transforms which stored for whole hierarchy
StructuredBuffer<GDRGPUMaterial> MaterialPool : register(GDRGPUMaterialPoolSlot);        // SRV: Data with materials

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
	float4 col = Shade(input.pos, input.normal, indices.ObjectMaterialIndex);
	return col;
}
