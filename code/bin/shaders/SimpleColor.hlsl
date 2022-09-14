#include "shared_structures.h"

cbuffer GlobalValues : register (b0)
{
  float4x4 VP; // camera view-proj
  float3 CameraPos; // Camera position
  float time; // Time in seconds
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
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VSOut VS(VSIn input)
{
    VSOut output;
    ObjectTransform myTransform = ObjectTransformData[indices.ObjectTransformIndex];

    output.pos = mul(VP, mul(myTransform.transform, float4(input.pos, 1.0)));
    output.color = float4(input.normal, 1.0);
    output.color.x += sin(time);

    return output;
}

float4 PS(VSOut input) : SV_TARGET
{
    return input.color;
}