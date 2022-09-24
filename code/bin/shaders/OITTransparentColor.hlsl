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
StructuredBuffer<LightSource> LightSourcesPool  : register(t7);        // SRV: Data with lights

RWStructuredBuffer<OITList> OITHeads : register(u1);                     // UAV: Data with Order Independent transparency lists
RWStructuredBuffer<OITNode> OITPool : register(u2);                     // UAV: Data with Order Independent transparency nodes

// add all shade functions
#include "ShadeFunctions.h"

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

[earlydepthstencil]
float4 PS(VSOut input) : SV_TARGET
{
    ObjectMaterial myMaterial = ObjectMaterialData[indices.ObjectMaterialIndex];
    uint2 screen_pos = uint2(input.pos.x - 0.5, input.pos.y - 0.5);
    float4 col = Shade(normalize(input.normal.xyz), input.unmodifiedPos.xyz, input.uv, myMaterial);
    uint new_element_index = OITPool.IncrementCounter();
    
    if (col.a != 0 && new_element_index < MAX_AMOUNT_OF_TRANSPARENT_PIXELS)
    {
      uint prevHead = 0xFFFFFFFF;

      InterlockedExchange(OITHeads[screen_pos.y * globals.width + screen_pos.x].RootIndex, new_element_index, prevHead);

      // add element to the beginning of the list
      OITPool[new_element_index].NextNodeIndex = prevHead;
      OITPool[new_element_index].Depth = input.pos.z;
      OITPool[new_element_index].Color = col;
    }

    return float4(0, 0, 0, 0); // draw nothing
}