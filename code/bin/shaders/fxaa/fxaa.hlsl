#include "shared/cpu_gpu_shared.h"

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 15
#include "fxaa/Fxaa3_11.h"

cbuffer GlobalValues : register (GDRGPUGlobalDataConstantBufferSlot)
{
  GDRGPUGlobalData globals;
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

float4 PS(VSOut input) : SV_TARGET
{
  float3 Data;

  FxaaTex FXAATexture = { LinearSampler, Source };

  if (globals.IsFXAA)
  {
	  // FXAA 3_11
	  Data = FxaaPixelShader(
		  input.uv, // pos
		  0, // fxaaConsolePosPos
		  FXAATexture, // tex
		  FXAATexture, // texLuma
		  FXAATexture, // fxaaConsole360TexExpBiasNegOne
		  FXAATexture, // fxaaConsole360TexExpBiasNegTwo
		  float2(1.0 / globals.Width, 1.0 / globals.Height), // fxaaQualityRcpFrame
		  0, // fxaaConsoleRcpFrameOpt
		  0, // fxaaConsoleRcpFrameOpt2
		  0, // fxaaConsole360RcpFrameOpt2
		  0.75, // fxaaQualitySubpix
		  0.166, // fxaaQualityEdgeThreshold
		  0.0833, // fxaaQualityEdgeThresholdMin
		  0, // fxaaConsoleEdgeSharpness
		  0, // fxaaConsoleEdgeThreshold
		  0, // fxaaConsoleEdgeThresholdMin
		  0 // fxaaConsole360ConstDir
	  );
  }
  else
  {
	  Data = Source.Sample(LinearSampler, input.uv).rgb;
  }

  return float4(Data, 1);
}