#pragma once
// This module depends on
//
// NodeTransformPool - which is a pool of GDRGPUNodeTransform
// ObjectTransformPool - which is a pool of GDRGPUObjectTransform
// GlobalValues - which is a constant buffer of GDRGPUGlobalData
// BoneMappingPool - which is a pool of GDRGPUBoneMapping
#ifndef __cplusplus

#include "shared/normal_matrix.h"

struct VSIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	float3 tangent : TANGENT;
	uint4 bones_indices : BONES_ID;
	float4 bones_weights : BONES_WEIGHT;
};

struct VSOut
{
	float4 pos : SV_POSITION;
	float4 worldPos : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float2 uv : TEXCOORD;
};

VSOut ProcessVSIn(VSIn input)
{
    VSOut output;
    float4x4 nodeTransform = 0;
    output.worldPos = (0).xxxx;

    // if we have bones, process them first
    if (indices.BoneMappingIndex != NONE_INDEX &&
          (input.bones_indices.x != NONE_INDEX ||
           input.bones_indices.y != NONE_INDEX ||
           input.bones_indices.z != NONE_INDEX ||
           input.bones_indices.w != NONE_INDEX))
    {
        if (input.bones_indices.x != NONE_INDEX && BoneMappingPool[indices.BoneMappingIndex].BoneMapping[input.bones_indices.x] != NONE_INDEX)
        {
          uint ind = BoneMappingPool[indices.BoneMappingIndex].BoneMapping[input.bones_indices.x];
          nodeTransform += 
            input.bones_weights.x * 
            mul(NodeTransformPool[ind].GlobalTransform, NodeTransformPool[ind].BoneOffset);
        }
        if (input.bones_indices.y != NONE_INDEX && BoneMappingPool[indices.BoneMappingIndex].BoneMapping[input.bones_indices.y] != NONE_INDEX)
        {
          uint ind = BoneMappingPool[indices.BoneMappingIndex].BoneMapping[input.bones_indices.y];
          nodeTransform +=
            input.bones_weights.y *
            mul(NodeTransformPool[ind].GlobalTransform, NodeTransformPool[ind].BoneOffset);
        }
        if (input.bones_indices.z != NONE_INDEX && BoneMappingPool[indices.BoneMappingIndex].BoneMapping[input.bones_indices.z] != NONE_INDEX)
        {
          uint ind = BoneMappingPool[indices.BoneMappingIndex].BoneMapping[input.bones_indices.z];
          nodeTransform +=
            input.bones_weights.z *
            mul(NodeTransformPool[ind].GlobalTransform, NodeTransformPool[ind].BoneOffset);
        }
        if (input.bones_indices.w != NONE_INDEX && BoneMappingPool[indices.BoneMappingIndex].BoneMapping[input.bones_indices.w] != NONE_INDEX)
        {
          uint ind = BoneMappingPool[indices.BoneMappingIndex].BoneMapping[input.bones_indices.w];
          nodeTransform +=
            input.bones_weights.w *
            mul(NodeTransformPool[ind].GlobalTransform, NodeTransformPool[ind].BoneOffset);
        }
        output.worldPos = mul(nodeTransform, float4(input.pos, 1.0));
        float4x4 normalMatr = GetNormalMatrix(nodeTransform);
        output.normal = mul(normalMatr, float4(input.normal, 1.0));
        output.tangent = mul(normalMatr, float4(input.tangent, 1.0));
    }
    else
    {
        output.worldPos = float4(input.pos, 1.0);
        output.normal = float4(input.normal, 1.0);
        output.tangent = float4(input.tangent, 1.0);
    }

    // then process object transform
    if (indices.ObjectTransformIndex != NONE_INDEX)
    {
        output.worldPos = mul(ObjectTransformPool[indices.ObjectTransformIndex].Transform, float4(output.worldPos.xyz, 1.0));
        float4x4 normalMatr = GetNormalMatrix(ObjectTransformPool[indices.ObjectTransformIndex].Transform);
        output.normal = mul(normalMatr, float4(output.normal.xyz, 1.0));
        output.tangent = mul(normalMatr, float4(output.tangent.xyz, 1.0));
    }
    
    output.pos = mul(globals.VP, output.worldPos);
    output.uv = input.uv;

    return output;
}

#endif // __cplusplus