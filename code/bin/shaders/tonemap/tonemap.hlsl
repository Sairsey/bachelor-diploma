#include "shared/cpu_gpu_shared.h"

cbuffer GlobalValues : register (GDRGPUGlobalDataConstantBufferSlot)
{
  GDRGPUGlobalData globals;
}

cbuffer Luminance : register(GDRGPULuminanceConstantBufferSlot)
{
  GDRGPULuminanceVariables data;
}

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
  float4 Data;
  // Tone mapping
  if (globals.IsTonemap)
    Data = float4(TonemapFilmic(Source.Sample(LinearSampler, input.uv).rgb, data.ExposureAdapted), 1);
  else
    Data = float4(Source.Sample(LinearSampler, input.uv).rgb, 1);
  
  
  // Lum in 4-th component
  Data.a = dot(Data.xyz, float3(0.299, 0.587, 0.114));

  // SRGB
  Data.xyz = linear_to_srgb(Data.xyz);

  return Data;
}