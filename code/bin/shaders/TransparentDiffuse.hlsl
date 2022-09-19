#include "shared_structures.h"
#include "RandomGenerator.h"
#include "Matrices.h"

cbuffer GlobalValues : register (b0)
{
  GlobalData globals;
}

cbuffer Indices : register(b1)
{
  ObjectIndices indices;
}

StructuredBuffer<ObjectTransform> ObjectTransformData : register(t2);    // SRV: Data with transforms which stored per object
StructuredBuffer<ObjectMaterial> ObjectMaterialData : register(t3);      // SRV: Data with materials which stored per object
Texture2D TexturesPool[]  : register(t4, space1);                        // Bindless Pool with all textures
SamplerState LinearSampler : register(s0);                               // Texture sampler

struct VSIn
{
  float3 pos : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
};

struct VSOut
{
  float4 pos : SV_POSITION;
  float4 unmodifiedPos : POSITION;
  float4 normal : NORMAL;
  float2 uv: TEXCOORD;
};

VSOut VS(VSIn input)
{
    VSOut output;
    ObjectTransform myTransform = ObjectTransformData[indices.ObjectTransformIndex];

    output.pos = mul(globals.VP, mul(myTransform.transform, float4(input.pos, 1.0)));
    output.unmodifiedPos = mul(myTransform.transform, float4(input.pos, 1.0));
    output.normal = mul(transpose(inverse(myTransform.transform)), float4(input.normal, 1.0));
    output.uv = input.uv;

    return output;
}


float4 Shade(float3 Normal, float3 Position, float2 uv, ObjectMaterial material)
{
  float3 Phong = float3(0, 0, 0);
  float alpha = 1.0;

  if (material.KdMapIndex != -1)
  {
    Phong = TexturesPool[material.KdMapIndex].Sample(LinearSampler, uv).xyz;
    alpha = TexturesPool[material.KdMapIndex].Sample(LinearSampler, uv).w;
  }

  return float4(Phong, alpha);
}

float4 PS(VSOut input) : SV_TARGET
{
    ObjectMaterial myMaterial = ObjectMaterialData[indices.ObjectMaterialIndex];
    float4 col = Shade(normalize(input.normal.xyz), input.unmodifiedPos.xyz, input.uv, myMaterial);
    if (0)
    {
      /*Random Color*/
      NumberGenerator N;
      N.SetSeed(indices.ObjectTransformIndex);
      col = float4(N.GetRandomFloat(0, 1), N.GetRandomFloat(0, 1), N.GetRandomFloat(0, 1), 1);
    }
    return col;
}