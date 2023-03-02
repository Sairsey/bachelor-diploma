#include "shared/cpu_gpu_shared.h"

cbuffer GlobalValues : register (GDRGPUGlobalDataConstantBufferSlot)
{
  GDRGPUGlobalData globals;
}

Texture2D<UINT> OITTexture : register(GDRGPUOITTextureSRVSlot);            // UAV: Headers of lists
StructuredBuffer<GDRGPUOITNode> OITPool : register(GDRGPUOITPoolSRVSlot);  // UAV: Elements of lists

SamplerState LinearSampler : register(GDRGPULinearSamplerSlot);  // Linear texture sampler
SamplerState NearestSampler : register(GDRGPUNearestSamplerSlot); // Nearest texture sampler

struct VSIn
{
  float3 pos : POSITION;
  float2 uv : TEXCOORD;
};

struct VSOut
{
  float4 pos : SV_POSITION;
  float2 uv : TEXCOORD;
};

VSOut VS(VSIn input)
{ 
    VSOut output;

    output.pos = float4(input.pos, 1.0);
    output.uv = input.uv;
    return output;
}

bool compare(GDRGPUOITNode a, GDRGPUOITNode b)
{
  float delta = abs(a.Depth) - abs(b.Depth);
  if (abs(delta) <= 1e-6)
    return a.Depth < b.Depth;
  return delta < 0;
}

float4 PS(VSOut input) : SV_TARGET
{
  uint2 screen_pos = uint2(input.pos.x - 0.5, input.pos.y - 0.5);
  GDRGPUOITNode nearest;
  GDRGPUOITNode nearest2;
  GDRGPUOITNode nearest3;
  GDRGPUOITNode nearest4;
  uint arraySize = 0;

  uint OITTexW, OITTexH;
  OITTexture.GetDimensions(OITTexW, OITTexH);
  if (screen_pos.x < 0 || screen_pos.x >= OITTexW || screen_pos.y < 0 || screen_pos.y >= OITTexH)
    return float4(0, 0, 0, 0);

  uint OITNumNodes, dummy;
  OITPool.GetDimensions(OITNumNodes, dummy);

  uint n = OITTexture[screen_pos];
  if (n == NONE_INDEX || n >= OITNumNodes)
    return float4(0, 0, 0, 0);

  if (globals.DebugOIT)
  {
    arraySize++;
    while (n != NONE_INDEX && n < OITNumNodes)
    {
      arraySize++;

      n = OITPool[n].NextNodeIndex;
    }

    if (arraySize == 1)
      return float4(0, 0, 0, 1);
    if (arraySize == 2)
      return float4(0, 0, 1, 1);
    if (arraySize == 3)
      return float4(0, 1, 0, 1);
    if (arraySize == 4)
      return float4(0, 1, 1, 1);
    if (arraySize == 5)
      return float4(1, 0, 0, 1);
    if (arraySize == 6)
      return float4(1, 0, 1, 1);
    if (arraySize == 6)
      return float4(1, 1, 0, 1);
    if (arraySize >= 7)
      return float4(1, 1, 1, 1);
  }
  // Out color splitted in 3 parts
  // 1) Opaque Color (stored in RT)
  // 2) Sorted Transparent Color
  // 3) Approximated WBOIT color
  // we use WBOIT color as starting color, and starting alpha

  // first pass -> we find nearest Opaque

  // calculate WBOIT and sorted
  float3 WBOITColor = float3(0, 0, 0);
  float divider = 0.0;
  float WBOITAlpha = 1;

  // fill nearests with default data
  nearest.Color = float4(0, 0, 0, 0);
  nearest.Depth = -1.0f;
  nearest2 = nearest;
  nearest3 = nearest;
  nearest4 = nearest;
  n = OITTexture[screen_pos];

  while (n != NONE_INDEX && n < OITNumNodes)
  {
    if (OITPool[n].Color.a == 1 && compare(OITPool[n], nearest))
      nearest = OITPool[n];
    n = OITPool[n].NextNodeIndex;
  }

  n = OITTexture[screen_pos];

  while (n != NONE_INDEX && n < OITNumNodes)
  {
    if (compare(OITPool[n], nearest))
    {
      nearest4 = nearest3;
      nearest3 = nearest2;
      nearest2 = nearest;
      nearest = OITPool[n];
    }
    else if (compare(OITPool[n], nearest2))
    {
      nearest4 = nearest3;
      nearest3 = nearest2;
      nearest2 = OITPool[n];
    }
    else if (compare(OITPool[n], nearest3))
    {
      nearest4 = nearest3;
      nearest3 = OITPool[n];
    }
    else if (compare(OITPool[n], nearest4))
    {
      nearest4 = OITPool[n];
    }

    WBOITColor += OITPool[n].Color.rgb * OITPool[n].Color.a;
    divider += OITPool[n].Color.a;
    WBOITAlpha *= 1 - OITPool[n].Color.a;
    arraySize++;

    n = OITPool[n].NextNodeIndex;
  }
  // remove nearest elements in WBOIT

  WBOITColor -= nearest.Color.rgb * nearest.Color.a +
    nearest2.Color.rgb * nearest2.Color.a +
    nearest3.Color.rgb * nearest3.Color.a +
    nearest4.Color.rgb * nearest4.Color.a;
  divider -= nearest.Color.a +
    nearest2.Color.a +
    nearest3.Color.a +
    nearest4.Color.a;

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
