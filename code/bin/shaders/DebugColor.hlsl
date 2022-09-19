#include "shared_structures.h"

cbuffer GlobalValues : register (b0)
{
  GlobalData globals;
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
    
    output.pos = mul(globals.VP, float4(input.pos, 1.0));

    return output;
}

float4 PS(VSOut input) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}