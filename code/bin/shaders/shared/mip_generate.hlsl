#include "shared/cpu_gpu_shared.h"

RWTexture2D<float> InputTexture : register(GDRGPUUserUnorderedAccess1Slot);
RWTexture2D<float> OutputTexture : register(GDRGPUUserUnorderedAccess2Slot);

cbuffer CB : register(GDRGPUUserConstantBuffer1Slot)
{
	int2 PrevWH;
	int2 WH;
}

[numthreads(32, 32, 1)]
void CS(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x > WH.x || DTid.y > WH.y)
		return;
	/*
	//DTid is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
	//As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
	float2 texcoords = TexelSize * (DTid.xy + 0.5);

	//The samplers linear interpolation will mix the four pixel values to the new pixels color
	float4 color = InputTexture.SampleLevel(SampleMax, texcoords, 0);

	//Write the final color into the destination texture.
	OutputTexture[DTid.xy] = color;
	*/
	uint width = PrevWH.x;
	uint height = PrevWH.y;

	int down_u = DTid.x * 2;
	int up_u = DTid.x * 2 + 1;
	int down_v = DTid.y * 2;
	int up_v = DTid.y * 2 + 1;

	//The samplers linear interpolation will mix the four pixel values to the new pixels color
	float4 color1 = InputTexture[int2(down_u, down_v)];
	float4 color2 = InputTexture[int2(down_u, up_v)];
	float4 color3 = InputTexture[int2(up_u, down_v)];
	float4 color4 = InputTexture[int2(up_u, up_v)];

	float4 color = max(max(color1, color2), max(color3, color4));

	if (width % 2 != 0)
	{
		int s_up_u = DTid.x * 2 + 2;

		//The samplers linear interpolation will mix the four pixel values to the new pixels color
		color1 = InputTexture[int2(s_up_u, down_v)];
		color2 = InputTexture[int2(s_up_u, up_v)];

		//Write the final color into the destination texture.
		color = max(color, max(color1, color2));
	}

	if (height % 2 != 0)
	{
		int s_up_v = DTid.y * 2 + 2;

		//The samplers linear interpolation will mix the four pixel values to the new pixels color
		color1 = InputTexture[int2(down_u, s_up_v)];
		color2 = InputTexture[int2(up_u, s_up_v)];

		//Write the final color into the destination texture.
		color = max(color, max(color1, color2));
	}

	if (height % 2 != 0 && width % 2 != 0)
	{
		color1 = InputTexture[int2(up_u + 1, up_v + 1)];
		color = max(color, max(color1, color2));
	}

	OutputTexture[DTid.xy] = color;
}