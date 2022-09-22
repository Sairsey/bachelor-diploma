#include "shared_structures.h"
#include "RandomGenerator.h"
#include "Matrices.h"
// this shader with indirect draw takes 10-12ms to compute. Maybe we can be faster, but idk
cbuffer GlobalValues : register (b0)
{
  GlobalData globals;
}

StructuredBuffer<OITList> OITHeads : register(t5);                       // SRV: Data with Order Independent transparency lists
StructuredBuffer<OITNode> OITPool : register(t6);                        // SRV: Data with Order Independent transparency nodes

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

    output.pos = float4(input.pos, 1.0);
    return output;
}

float weight(float depth, float a)
{
  return 1.0;
}

// Using here a "mixed" method
float4 PS(VSOut input) : SV_TARGET
{
    uint2 screen_pos = uint2(input.pos.x - 0.5, input.pos.y - 0.5);
    OITNode nearest;
    OITNode nearest2;
    OITNode nearest3;
    OITNode nearest4;
    uint arraySize = 0;
    uint n = OITHeads[screen_pos.y * globals.width + screen_pos.x].RootIndex;
    if (n == 0xFFFFFFFF)
      return float4(0, 0, 0, 0);

    // Out color splitted in 3 parts
    // 1) Opaque Color (stored in RT)
    // 2) Sorted Transparent Color
    // 3) Approximated WBOIT color
    // we use WBOIT color as starting color, and starting alpha

    // calculate WBOIT and sorted
    float3 WBOITColor = float3(0, 0, 0);
    float divider = 0.0;
    float WBOITAlpha = 1;

    // fill nearests with default data
    nearest.Color = float4(0, 0, 0, 0);
    nearest.Depth = 1.0;
    nearest2 = nearest;
    nearest3 = nearest;
    nearest4 = nearest;

    n = OITHeads[screen_pos.y * globals.width + screen_pos.x].RootIndex;
    //[unroll(MAX_TRANSPARENT_ARRAY_SIZE)]
    while (n != 0xFFFFFFFF && arraySize < MAX_TRANSPARENT_ARRAY_SIZE)
    {
      if (OITPool[n].Depth < nearest.Depth)
      {
        nearest4 = nearest3;
        nearest3 = nearest2;
        nearest2 = nearest;
        nearest = OITPool[n];
      }
      else if (OITPool[n].Depth < nearest2.Depth)
      {
        nearest4 = nearest3;
        nearest3 = nearest2;
        nearest2 = OITPool[n];
      }
      else if (OITPool[n].Depth < nearest3.Depth)
      {
        nearest4 = nearest3;
        nearest3 = OITPool[n];
      }
      else if (OITPool[n].Depth < nearest4.Depth)
      {
        nearest4 = OITPool[n];
      }
      
      WBOITColor += OITPool[n].Color.rgb * OITPool[n].Color.a * weight(OITPool[n].Depth, OITPool[n].Color.a); // Added additional a multiplier and now it is perfect
      divider += OITPool[n].Color.a * weight(OITPool[n].Depth, OITPool[n].Color.a);
      WBOITAlpha *= 1 - OITPool[n].Color.a;
      arraySize++;

      n = OITPool[n].NextNodeIndex;
    }
    // remove nearest elements in WBOIT

    WBOITColor -= nearest.Color.rgb * nearest.Color.a * weight(nearest.Depth, nearest.Color.a) +
      nearest2.Color.rgb * nearest2.Color.a * weight(nearest2.Depth, nearest2.Color.a) +
      nearest3.Color.rgb * nearest3.Color.a * weight(nearest3.Depth, nearest3.Color.a) +
      nearest4.Color.rgb * nearest4.Color.a * weight(nearest4.Depth, nearest4.Color.a);
    divider -= nearest.Color.a * weight(nearest.Depth, nearest.Color.a) +
      nearest2.Color.a * weight(nearest2.Depth, nearest2.Color.a) +
      nearest3.Color.a * weight(nearest3.Depth, nearest3.Color.a) +
      nearest4.Color.a * weight(nearest4.Depth, nearest4.Color.a);

    WBOITColor /= divider + 0.00001f;

    float ResultAlpha = WBOITAlpha;
    float3 ResultColor = 
      (((WBOITColor * 
      (1 - nearest4.Color.a) + nearest4.Color.a * nearest4.Color.rgb) *
      (1 - nearest3.Color.a) + nearest3.Color.a * nearest3.Color.rgb) *
      (1 - nearest2.Color.a) + nearest2.Color.a * nearest2.Color.rgb) *
      (1 - nearest.Color.a) + nearest.Color.a * nearest.Color.rgb;

    return float4(ResultColor, 1.0 - ResultAlpha);
}