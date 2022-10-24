Texture2D InputTexture : register(t0);
RWTexture2D<float> OutputTexture : register(u1);
SamplerState SampleMax: register(s0);

cbuffer CB : register(b0)
{
  float2 TexelSize;	// 1.0 / destination dimension
	int2 WH;
}

[numthreads(32, 32, 1)]
void CS(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x > WH.x || DTid.y > WH.y)
		return;
	//DTid is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
	//As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
	float2 texcoords = TexelSize * (DTid.xy + 0.5);

	//The samplers linear interpolation will mix the four pixel values to the new pixels color
	float4 color = InputTexture.SampleLevel(SampleMax, texcoords, 0);

	//Write the final color into the destination texture.
	OutputTexture[DTid.xy] = color;
}