#include "shared_structures.h"

Texture2D Source: register(t0);
SamplerState LinearSampler : register(s0);  // Texture sampler
RWStructuredBuffer<LuminanceVariables> Luminance: register(u0);

cbuffer GlobalValues : register (b0)
{
  GlobalData globals;
}


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

static const float A = 0.15; // Shoulder Strength
static const float B = 0.50; // Linear Strength
static const float C = 0.10; // Linear Angle
static const float D = 0.20; // Toe Strength
static const float E = 0.02; // Toe Numerator
static const float F = 0.30; // Toe Denominator
                             // Note: E/F = Toe Angle
static const float W = 11.2; // Linear White Point Value

float3 Uncharted2Tonemap(float3 x)
{
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 TonemapFilmic(float3 color, float E)
{
  float exposure = globals.SceneExposure;
  float3 curr = Uncharted2Tonemap(exposure * E * color);
  float3 whiteScale = 1.0f / Uncharted2Tonemap(W);
  return curr * whiteScale;
}

float linear_to_srgb_one(float theLinearValue) {
  return theLinearValue <= 0.0031308f
    ? theLinearValue * 12.92f
    : pow(theLinearValue, 0.41666) * 1.055f - 0.055f;
}

float3 linear_to_srgb(float3 color) {
  return float3(linear_to_srgb_one(color.r), linear_to_srgb_one(color.g), linear_to_srgb_one(color.b));
}

float4 PS(VSOut input) : SV_TARGET
{
  //return float4(linear_to_srgb(Source.Sample(LinearSampler, input.uv).rgb), 1);
  return float4(linear_to_srgb(TonemapFilmic(Source.Sample(LinearSampler, input.uv).rgb, Luminance[0].ExposureAdapted)), 1);
}