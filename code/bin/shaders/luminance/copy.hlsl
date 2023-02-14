#include "shared/cpu_gpu_shared.h"

Texture2D Source: register(GDRGPUUserShaderResource1Slot);

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

float PS(VSOut input) : SV_TARGET
{
    return Source.Sample(LinearSampler, input.uv).r;
}