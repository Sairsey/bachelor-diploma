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
StructuredBuffer<ObjectMaterial> ObjectMaterialData : register(t3);      // SRV: Data with materials which stored per object
Texture2D TexturesPool[]  : register(t4, space1);                        // Bindless Pool with all textures
StructuredBuffer<LightSource> LightSourcesPool  : register(t5);        // SRV: Data with lights

#include "ShadeFunctions.h"

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
  float4 unmodifiedPos : POSITION;
  float4 normal : NORMAL;
  float3 tangent : TANGENT;
  float2 uv: TEXCOORD;
};

VSOut VS(VSIn input)
{
    VSOut output;
    ObjectTransform myTransform = ObjectTransformData[indices.ObjectTransformIndex];

    output.pos = mul(globals.VP, mul(myTransform.transform, float4(input.pos, 1.0)));
    output.unmodifiedPos = mul(myTransform.transform, float4(input.pos, 1.0));
    output.normal = mul(GetNormalMatrix(myTransform.transform), float4(input.normal, 1.0));
    output.tangent = mul(GetNormalMatrix(myTransform.transform), float4(input.tangent, 1.0));
    output.uv = input.uv;

    return output;
}

float4 PS(VSOut input) : SV_TARGET
{
    ObjectMaterial myMaterial = ObjectMaterialData[indices.ObjectMaterialIndex];
    
    float3 Normal = normalize(input.normal.xyz);
    float3 Tangent = normalize(input.tangent.xyz);
    float3 Bitangent = cross(Normal, Tangent);
    float3x3 TBN = transpose(float3x3(Tangent, Bitangent, Normal));
    
    if (myMaterial.NormalMapIndex != -1)
    {
      Normal = TexturesPool[myMaterial.NormalMapIndex].Sample(LinearSampler, input.uv).xyz;
      Normal = (Normal * 2.0 - 1.0);
      Normal = normalize(mul(TBN, Normal));
    }

    float4 col = Shade(Normal, input.unmodifiedPos.xyz, input.uv, myMaterial);
    if (0)
    {
      /*Random Color*/
      NumberGenerator N;
      N.SetSeed(indices.ObjectTransformIndex);
      col = float4(N.GetRandomFloat(0, 1), N.GetRandomFloat(0, 1), N.GetRandomFloat(0, 1), 1);
    }
    return col;
}