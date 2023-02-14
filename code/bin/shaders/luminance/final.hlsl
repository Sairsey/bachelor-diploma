#include "shared/cpu_gpu_shared.h"

cbuffer GlobalValues : register (GDRGPUGlobalDataConstantBufferSlot)
{
  GDRGPUGlobalData globals;
}

SamplerState LinearSampler : register(GDRGPULinearSamplerSlot);  // Linear texture sampler
SamplerState NearestSampler : register(GDRGPUNearestSamplerSlot); // Nearest texture sampler

Texture2D Source: register(GDRGPUUserShaderResource1Slot);
RWStructuredBuffer<GDRGPULuminanceVariables> Luminance: register(GDRGPULuminanceUnorderedAccessBufferSlot);

bool IsNaN(float x)
{
  return !(x < 0.f || x > 0.f || x == 0.f);
}

[numthreads(1, 1, 1)]
void CS(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    float value = 0;
    for (int i = 0; i < max(globals.Height * 0.03125, 1); i++)
      for (int j = 0; j < max(globals.Width * 0.03125, 1); j++)
        value += max(Source[int2(j, i)].r, 0);

    value /= max(globals.Height * 0.03125, 1);
    value /= max(globals.Width * 0.03125, 1);

    float averageLuminance = exp(value) - 1;
    float averageLuminanceAdapted = Luminance[0].LuminanceAdapted + (averageLuminance - Luminance[0].LuminanceAdapted) * (1.0 - exp(-globals.DeltaTime));

    float keyValue = 1.03f - 2.f / (2.f + log10(averageLuminance + 1));
    Luminance[0].Luminance = averageLuminance;
    Luminance[0].Exposure = keyValue / averageLuminance;

    if (averageLuminanceAdapted > 0.0001)
    {
        Luminance[0].LuminanceAdapted = averageLuminanceAdapted;
        Luminance[0].ExposureAdapted = keyValue / averageLuminanceAdapted;
    }
    else
    {
        Luminance[0].LuminanceAdapted = Luminance[0].Luminance;
        Luminance[0].ExposureAdapted = Luminance[0].Exposure;
    }
}