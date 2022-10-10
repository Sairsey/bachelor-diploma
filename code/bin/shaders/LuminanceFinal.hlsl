#include "shared_structures.h"

Texture2D Source: register(t0);
SamplerState LinearSampler : register(s0);  // Texture sampler
RWStructuredBuffer<LuminanceVariables> Luminance: register(u0);

cbuffer GlobalValues : register (b0)
{
  GlobalData globals;
}

bool IsNaN(float x)
{
  return !(x < 0.f || x > 0.f || x == 0.f);
}

[numthreads(1, 1, 1)]
void CS(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    float value = 0;
    for (int i = 0; i < max(globals.height * 0.03125, 1); i++)
      for (int j = 0; j < max(globals.width * 0.03125, 1); j++)
        value += Source[int2(i, j)].r;

    value /= max(globals.height * 0.03125, 1);
    value /= max(globals.width * 0.03125, 1);

    float averageLuminance = exp(value) - 1;
    float averageLuminanceAdapted = Luminance[0].LuminanceAdapted + (averageLuminance - Luminance[0].LuminanceAdapted) * (1.0 - exp(-globals.DeltaTime));

    averageLuminanceAdapted = max(averageLuminanceAdapted, 0.0001);

    float keyValue = 1.f - 2.f / (2.f + log10(averageLuminance + 1));
    float keyValueAdapted = 1.f - 2.f / (2.f + log10(averageLuminanceAdapted + 1));

    Luminance[0].Luminance = averageLuminance;
    Luminance[0].LuminanceAdapted = averageLuminanceAdapted;
    Luminance[0].Exposure = keyValue / averageLuminance;
    Luminance[0].ExposureAdapted = keyValue / averageLuminanceAdapted;
}