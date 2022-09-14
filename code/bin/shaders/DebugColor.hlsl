#include "shared_structures.h"

cbuffer GlobalValues : register (b0)
{
  float4x4 VP; // camera view-proj
  float3 CameraPos; // Camera position
  float time; // Time in seconds
}

struct VSIn
{
  float3 pos : POSITION;
};

struct VSOut
{
    float4 pos : SV_POSITION;
};

VSOut VS(VSIn input)
{
    VSOut output;
    
    output.pos = mul(VP, float4(input.pos, 1.0));

    return output;
}

float4 PS(VSOut input) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}