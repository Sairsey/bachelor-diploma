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
    OITNode frags[MAX_TRANSPARENT_ARRAY_SIZE];
    uint arraySize = 0;
    uint n = OITHeads[screen_pos.y * globals.width + screen_pos.x].RootIndex;
    while (n != 0xFFFFFFFF && arraySize < MAX_TRANSPARENT_ARRAY_SIZE)
    {
      frags[arraySize] = OITPool[n];
      n = frags[arraySize].NextNodeIndex;
      arraySize++;
    }

    if (arraySize == 0)
      return float4(0, 0, 0, 0);

    // lets sort with selection sort first MAX_TRANSPARENT_DEPTH elements
    uint sorted_amount = arraySize;
    if (sorted_amount > MAX_SORTED_PIXELS_AMOUNT)
      sorted_amount = MAX_SORTED_PIXELS_AMOUNT;

    [unroll(MAX_SORTED_PIXELS_AMOUNT)]
    for (uint i = 0; i < sorted_amount; i++)
    {
      uint min = arraySize - 1;
      for (uint j = arraySize - 1; j > i; j--)
        if (frags[min].Depth > frags[j].Depth)
          min = j;

      // swap i and min
      if (frags[min].Depth < frags[i].Depth)
      {
        OITNode tmp = frags[i];
        frags[i] = frags[min];
        frags[min] = tmp;
      }
    }

    // now 0 is the smallest by depth value and that means it is nearest

    // reverse first MAX_TRANSPARENT_DEPTH, so we start from farthest in sorted_amount
    [unroll(MAX_SORTED_PIXELS_AMOUNT)]
    for (uint i = 0; i < sorted_amount / 2; i++)
    {
      OITNode tmp = frags[i];
      frags[i] = frags[sorted_amount - i - 1];
      frags[sorted_amount - i - 1] = tmp;
    }

    // Out color splitted in 3 parts
    // 1) Opaque Color (stored in RT)
    // 2) Sorted Transparent Color
    // 3) Approximated WBOIT color
    // we use WBOIT color as starting color, and starting alpha

    // calculate WBOIT
    float3 WBOITColor = float3(0, 0, 0);
    float divider = 0.0;
    float WBOITAlpha = 1;
    [unroll(MAX_TRANSPARENT_ARRAY_SIZE - MAX_SORTED_PIXELS_AMOUNT)]
    for (uint i = sorted_amount; i < arraySize; i++)
    {
      WBOITColor += frags[i].Color.rgb * frags[i].Color.a * weight(frags[i].Depth, frags[i].Color.a);
      divider += frags[i].Color.a * weight(frags[i].Depth, frags[i].Color.a);
      WBOITAlpha *= 1 - frags[i].Color.a;
    }
    if (divider != 0.0)
      WBOITColor /= divider;
    WBOITAlpha = WBOITAlpha;

    float3 SortedColor = WBOITColor;
    float SortedAlpha = WBOITAlpha;
    
    for (uint i = 0; i < sorted_amount; i++)
    {
      SortedColor = SortedColor * (1 - frags[i].Color.a) + frags[i].Color.a * frags[i].Color.rgb;
      SortedAlpha *= (1 - frags[i].Color.a);
    }

    SortedAlpha = 1.0 - SortedAlpha;
    return float4(SortedColor, SortedAlpha);
}