#include "shared_structures.h"
#include "RandomGenerator.h"
#include "Matrices.h"
#include "NormalMatrix.h"

cbuffer GlobalValues : register (b0)
{
  GlobalData globals;
}

cbuffer Indices : register(b1)
{
  ObjectIndices indices;
}

StructuredBuffer<ObjectTransform> ObjectTransformData : register(t0);    // SRV: Data with transforms which stored per object


struct VSIn
{
  float3 pos : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
  float3 tangent : TANGENT;
  int4 bones_indices : BONES_ID;
  float4 bones_weights : BONES_WEIGHT;
};

struct VSOut
{
  float4 pos : SV_POSITION;
};

VSOut VS(VSIn input)
{
    VSOut output;
    float4x4 myTransform = 0;
    float4 unmodifiedPos = (0).xxxx;

    if (indices.ObjectParams & OBJECT_PARAMETER_SKINNED)
    {
      if (input.bones_weights.x != 0)
        myTransform += input.bones_weights.x * mul(ObjectTransformData[input.bones_indices.x].transform, ObjectTransformData[input.bones_indices.x].transformBoneOffset);
      if (input.bones_weights.y != 0)                                                               
        myTransform += input.bones_weights.y * mul(ObjectTransformData[input.bones_indices.y].transform, ObjectTransformData[input.bones_indices.y].transformBoneOffset);
      if (input.bones_weights.z != 0)                                                                
        myTransform += input.bones_weights.z * mul(ObjectTransformData[input.bones_indices.z].transform, ObjectTransformData[input.bones_indices.z].transformBoneOffset);
      if (input.bones_weights.w != 0)                                                                
        myTransform += input.bones_weights.w * mul(ObjectTransformData[input.bones_indices.w].transform, ObjectTransformData[input.bones_indices.w].transformBoneOffset);
    }
    else
    {
      myTransform = ObjectTransformData[indices.ObjectTransformIndex].transform;
    }

    unmodifiedPos = mul(myTransform, float4(input.pos, 1.0));
    
    output.pos = mul(globals.VP, unmodifiedPos);
    
    return output;
}