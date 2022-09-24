SamplerState LinearSampler : register(s0);  // Texture sampler
SamplerState NearestSampler : register(s1); // Texture sampler

float4 ShadeDiffuse(float3 Normal, float3 Position, float2 uv, ObjectMaterial material)
{
  float3 Phong = float3(0, 0, 0);
  float alpha = 1.0;

  Phong = material.Kd;
  if (material.KdMapIndex != -1)
  {
    Phong = TexturesPool[material.KdMapIndex].Sample(LinearSampler, uv).xyz;
    alpha = TexturesPool[material.KdMapIndex].Sample(NearestSampler, uv).w;
  }

  return float4(Phong, alpha);
}

float4 ShadePhong(float3 Normal, float3 Position, float2 uv, ObjectMaterial material)
{
  // HARDCODED
  float3 L = float3(1, 1, 1);
  L = normalize(L);

  float3 V = globals.CameraPos - Position;
  V = normalize(V);

  float3 Phong = material.Ka;
  float alpha = 1.0;

  float NdotL = dot(Normal, L);


  if (NdotL > 0) {
    float3 R = reflect(-L, Normal);

    /* Diffuse color*/
    float3 diffuseColor = float4(material.Kd, 0);
    if (material.KdMapIndex != -1)
    {
      diffuseColor = TexturesPool[material.KdMapIndex].Sample(LinearSampler, uv).xyz;
      alpha = TexturesPool[material.KdMapIndex].Sample(NearestSampler, uv).w;
    }
    Phong += (diffuseColor * NdotL);

    /*Specular color*/
    float RdotV = dot(R, V);
    Phong += material.Ks * pow(max(0.0f, RdotV), material.Ph);
  }

  return float4(Phong, alpha);
}

float4 Shade(float3 Normal, float3 Position, float2 uv, ObjectMaterial material)
{
  switch (material.ShadeType)
  {
    case MATERIAL_SHADER_DIFFUSE:
      return ShadeDiffuse(Normal, Position, uv, material);
      break;
    case MATERIAL_SHADER_PHONG:
      return ShadePhong(Normal, Position, uv, material);
      break;
    default:
      return float4(1, 0, 1, 1);
  }
}

