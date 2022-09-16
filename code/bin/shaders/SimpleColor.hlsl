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
StructuredBuffer<ObjectMaterial> ObjectMaterialData : register(t3);      // SRV: Data with materials which stored per object

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

    output.pos = mul(VP, mul(myTransform.transform, float4(input.pos, 1.0)));
    output.unmodifiedPos = mul(myTransform.transform, float4(input.pos, 1.0));
    output.normal = mul(myTransform.transformInversedTransposed, float4(input.normal, 1.0));
    output.uv = input.uv;

    return output;
}


float3 Shade(float3 Normal, float3 Position, float2 uv, ObjectMaterial material)
{
  // HARDCODED
  float3 L = float3(1, 1, 1);
  L = normalize(L);

  float3 V = CameraPos - Position;
  V = normalize(V);

  float3 Phong = material.Ka;

  float NdotL = dot(Normal, L);

  if (NdotL > 0) {
    float3 R = reflect(-L, Normal);

    /* Diffuse color*/
    float3 diffuseColor = float4(material.Kd, 0);
    Phong += (diffuseColor * NdotL);

    /*Specular color*/
    float RdotV = dot(R, V);
    Phong += material.Ks * pow(max(0.0f, RdotV), material.Ph);
  }

  return Phong;
}

float4 PS(VSOut input) : SV_TARGET
{
    ObjectMaterial myMaterial = ObjectMaterialData[indices.ObjectMaterialIndex];
    float4 col = float4(Shade(normalize(input.normal.xyz), input.unmodifiedPos.xyz, input.uv, myMaterial), 1);
    return col;
}