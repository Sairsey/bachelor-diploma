#include "shared/cpu_gpu_shared.h"

cbuffer GlobalValues : register(GDRGPUGlobalDataConstantBufferSlot)
{
	GDRGPUGlobalData globals;
}

cbuffer Indices : register(GDRGPUObjectIndicesConstantBufferSlot)
{
	GDRGPUObjectIndices indices;
}
StructuredBuffer<GDRGPUObjectTransform> ObjectTransformPool : register(GDRGPUObjectTransformPoolSlot);  // SRV: Data with transforms which stored per object
StructuredBuffer<GDRGPUNodeTransform> NodeTransformPool : register(GDRGPUNodeTransformPoolSlot);        // SRV: Data with transforms which stored for whole hierarchy
StructuredBuffer<GDRGPUBoneMapping> BoneMappingPool : register(GDRGPUBoneMappingSlot);                  // SRV: Data with bone mappings

#include "shared/vertex_process.h"

VSOut VS(VSIn input)
{
	VSOut output = ProcessVSIn(input);
	return output;
}