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

StructuredBuffer<ObjectTransform> ObjectTransformData : register(t2);    // SRV: Data with transforms which stored per object

struct VSIn
{
  float3 pos : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
  float3 tangent : TANGENT;
};

struct VSOut
{
  float4 pos : SV_POSITION;
};

VSOut VS(VSIn input)
{
    VSOut output;
    ObjectTransform myTransform = ObjectTransformData[indices.ObjectTransformIndex];

    output.pos = mul(globals.VP, mul(myTransform.transform, float4(input.pos, 1.0)));
    return output;
}

[earlydepthstencil]
uint PS(VSOut input) : SV_TARGET
{
    return indices.ObjectIndex;
}