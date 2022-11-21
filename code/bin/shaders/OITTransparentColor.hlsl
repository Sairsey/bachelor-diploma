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
StructuredBuffer<ObjectMaterial> ObjectMaterialData : register(t1);      // SRV: Data with materials which stored per object
Texture2D TexturesPool[]  : register(t2, space1);                        // Bindless Pool with all textures
StructuredBuffer<LightSource> LightSourcesPool  : register(t3);        // SRV: Data with lights
TextureCube CubeTexturesPool[] : register(t4, space2);                   // Bindless Pool with all textures

RWStructuredBuffer<OITList> OITHeads : register(u0);                     // UAV: Data with Order Independent transparency lists
RWStructuredBuffer<OITNode> OITPool : register(u1);                     // UAV: Data with Order Independent transparency nodes


// add all shade functions
#include "ShadeFunctions.h"

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
  float4 unmodifiedPos : POSITION;
  float4 normal : NORMAL;
  float3 tangent : TANGENT;
  float2 uv: TEXCOORD;
};

VSOut VS(VSIn input)
{
    VSOut output;
    float4x4 myTransform = 0;
    output.unmodifiedPos = (0).xxxx;

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

    output.unmodifiedPos = mul(myTransform, float4(input.pos, 1.0));
    
    output.pos = mul(globals.VP, output.unmodifiedPos);
    output.normal = mul(GetNormalMatrix(myTransform), float4(input.normal, 1.0));
    output.tangent = mul(GetNormalMatrix(myTransform), float4(input.tangent, 1.0));

    output.uv = input.uv;

    return output;
}

[earlydepthstencil]
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

    float val = dot(Normal, globals.CameraPos - input.unmodifiedPos.xyz);

    Normal *= val / abs(val);
    uint2 screen_pos = uint2(input.pos.x - 0.5, input.pos.y - 0.5);
    float4 col = Shade(Normal, input.unmodifiedPos.xyz, input.uv, myMaterial);
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