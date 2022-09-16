#include "source/SharedStructures.h"

cbuffer GlobalValues : register (b0)
{
    float4x4 VP; // camera view-proj
    float3 CameraPos; // Camera position
    float time; // Time in seconds
}

cbuffer Indices : register(b1)
{
    ObjectIndices indices;
}

StructuredBuffer<ObjectTransform> ObjectTransformData : register(t0);    // SRV: Data with transforms which stored per object
StructuredBuffer<ObjectMaterial> ObjectMaterialData : register(t3);      // SRV: Data with materials which stored per object
Texture2D TexturesPool[]  : register(t2, space1);                        // Bindless Pool with all textures
SamplerState LinearSampler : register(s0);                               // Texture sampler


#define RANDOM_IA 16807
#define RANDOM_IM 2147483647
#define RANDOM_AM (1.0f/float(RANDOM_IM))
#define RANDOM_IQ 127773u
#define RANDOM_IR 2836
#define RANDOM_MASK 123459876

struct NumberGenerator {
    int seed; // Used to generate values.

    // Returns the current random float.
    float GetCurrentFloat() {
        Cycle();
        return RANDOM_AM * seed;
    }

    // Returns the current random int.
    int GetCurrentInt() {
        Cycle();
        return seed;
    }

    // Generates the next number in the sequence.
    void Cycle() {
        seed ^= RANDOM_MASK;
        int k = seed / RANDOM_IQ;
        seed = RANDOM_IA * (seed - k * RANDOM_IQ) - RANDOM_IR * k;

        if (seed < 0)
            seed += RANDOM_IM;

        seed ^= RANDOM_MASK;
    }

    // Cycles the generator based on the input count. Useful for generating a thread unique seed.
    // PERFORMANCE - O(N)
    void Cycle(const uint _count) {
        for (uint i = 0; i < _count; ++i)
            Cycle();
    }

    // Returns a random float within the input range.
    float GetRandomFloat(const float low, const float high) {
        float v = GetCurrentFloat();
        return low * (1.0f - v) + high * v;
    }

    // Sets the seed
    void SetSeed(const uint value) {
        seed = int(value);
        Cycle();
    }
};


struct VSOut
{
    float4 pos : SV_POSITION;
    float4 unmodifiedPos : POSITION;
    float4 normal : NORMAL;
    float2 uv: TEXCOORD;
};

VSOut VS(float3 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
    VSOut output;

    if (indices.ObjectTransformIndex == -1)
    {
        output.pos = mul(VP, float4(pos, 1.0));
        output.normal = float4(normal, 1.0);
        output.uv = uv;
    }
    else
    {
        ObjectTransform myTransform = ObjectTransformData[indices.ObjectTransformIndex];

        output.pos = mul(VP, mul(myTransform.transform, float4(pos, 1.0)));
        output.unmodifiedPos = mul(myTransform.transform, float4(pos, 1.0));
        output.normal = mul(myTransform.transformInversedTransposed, float4(normal, 1.0));
        output.uv = uv;
    }
    return output;
}

float CellFilter(float data)
{
    if (data > 0.75)
        data = 0.75;
    else if (data > 0.5)
        data = 0.5;
    else if (data > 0.25)
        data = 0.25;
    else if (data > 0)
        data = 0;
    return data;
}

float3 Shade(float3 Normal, float3 Position, float2 uv, ObjectMaterial material)
{
    // HARDCODED
    float3 L = float3(1, 1, 1);
    L = normalize(L);

    float3 V = CameraPos - Position;
    V = normalize(V);

    float3 Phong = material.Ka;

    float NdotL = dot(Normal, L);

    if (NdotL > 0) {
        float3 R = reflect(-L, Normal);

        if (material.isCellShaded)
            NdotL = CellFilter(NdotL);

        /* Diffuse color*/
        float3 diffuseColor = float4(material.Kd, 0);
        if (material.KdMapIndex != -1)
            diffuseColor = TexturesPool[material.KdMapIndex].Sample(LinearSampler, uv).xyz;
        Phong += (diffuseColor * NdotL);

        /*Specular color*/
        float RdotV = dot(R, V);
        Phong += !material.isCellShaded * material.Ks * pow(max(0.0f, RdotV), material.Ph);
    }	
	
    return Phong;
}

float4 PS(VSOut input) : SV_TARGET
{
    if (indices.ObjectTransformIndex == -1)
    {
        return float4(1, 1, 1, 1);
    }
    else
    {
		ObjectMaterial myMaterial = ObjectMaterialData[indices.ObjectMaterialIndex];
		float4 col = float4(Shade(normalize(input.normal.xyz), input.unmodifiedPos.xyz, input.uv, myMaterial), 1);
		if (myMaterial.isTransparent != 0)
			discard;

        /*NumberGenerator N;
        N.SetSeed(indices.ObjectTransformIndex);
        col = float4(N.GetRandomFloat(0, 1), N.GetRandomFloat(0, 1), N.GetRandomFloat(0, 1), 1);*/
		return col;
		// return float4(normalize(input.normal.xyz), 1);        
    }
}