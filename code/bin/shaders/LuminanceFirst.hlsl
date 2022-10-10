#include "shared_structures.h"

Texture2D Source: register(t0);
SamplerState LinearSampler : register(s0);  // Texture sampler

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

float PS(VSOut input) : SV_TARGET
{
  float3 color = Source.Sample(LinearSampler, input.uv).rgb;
  float value = log(0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b + 1.0);
  return value;
}