#include "shared_structures.h"
#include "RandomGenerator.h"
#include "Matrices.h"
#include "NormalMatrix.h"

cbuffer GlobalValues : register (b0)
{
  GlobalData globals;
}

TextureCube CubeTexturesPool[] : register(t0, space2);                        // Bindless Pool with all textures

SamplerState LinearSampler : register(s0);  // Texture sampler
SamplerState NearestSampler : register(s1); // Texture sampler

struct VSIn
{
  float3 pos : POSITION;
};

struct VSOut
{
  float4 pos : SV_POSITION;
  float3 TexCoords : TEXCOORDS;
};

VSOut VS(VSIn input)
{
    VSOut output;
    float4x4 transform = globals.VP;
    output.pos = mul(transform, float4(input.pos, 1)).xyww;
    output.TexCoords = normalize(input.pos);
    return output;
}

float4 PS(VSOut input) : SV_TARGET
{
  if (globals.SkyboxCubemapIndex == -1)
    return float4(0., 0., 0.0, 1);
  else
    return CubeTexturesPool[globals.SkyboxCubemapIndex].Sample(LinearSampler, input.TexCoords).rgba;
}