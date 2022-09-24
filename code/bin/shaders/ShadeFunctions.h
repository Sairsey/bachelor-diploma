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
  float3 resultColor = float3(0, 0, 0);
  float alpha = 1.0;
  float3 ambientColor = material.Ka;
  if (material.KaMapIndex != -1)
    ambientColor = TexturesPool[material.KaMapIndex].Sample(LinearSampler, uv).xyz
    ;
  float3 diffuseColor = material.Kd;
  if (material.KdMapIndex != -1)
  {
    diffuseColor = TexturesPool[material.KdMapIndex].Sample(LinearSampler, uv).xyz;
    alpha = TexturesPool[material.KdMapIndex].Sample(NearestSampler, uv).w;
  }

  float3 specularColor = material.Ks;
  if (material.KsMapIndex != -1)
    specularColor = TexturesPool[material.KsMapIndex].Sample(LinearSampler, uv).xyz;

  float3 V = globals.CameraPos - Position;
  V = normalize(V);

  resultColor = ambientColor;

  for (uint i = 0; i < globals.LightsAmount; i++)
  {
    if (LightSourcesPool[i].LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
    {
      float3 LightDir = float3(0, -1, 0);
      float4x4 TransformMatrix = ObjectTransformData[LightSourcesPool[i].ObjectTransformIndex].transform;
      TransformMatrix[0][3] = 0;
      TransformMatrix[1][3] = 0;
      TransformMatrix[2][3] = 0;
      LightDir = mul(TransformMatrix, float4(LightDir, 1.0)).xyz;
      LightDir = normalize(LightDir);
      float3 L = -LightDir;

      float NdotL = dot(Normal, L);

      if (NdotL > 0)
      {
        float3 R = reflect(-L, Normal);

        resultColor += LightSourcesPool[i].Color * diffuseColor * NdotL;

        float RdotV = dot(R, V);
        resultColor += LightSourcesPool[i].Color * specularColor * pow(max(0.0f, RdotV), material.Ph);
      }
    }
    else if (LightSourcesPool[i].LightSourceType == LIGHT_SOURCE_TYPE_POINT)
    {
      float3 LightPos = float3(0, 0, 0);
      float4x4 TransformMatrix = ObjectTransformData[LightSourcesPool[i].ObjectTransformIndex].transform;
      LightPos = mul(TransformMatrix, float4(LightPos, 1.0)).xyz;
      
      float3 L = LightPos - Position;
      float d = length(L);
      L = normalize(L);

      float attenuation = 1.0 / (
        LightSourcesPool[i].ConstantAttenuation +
        LightSourcesPool[i].LinearAttenuation * d +
        LightSourcesPool[i].QuadricAttenuation * d * d);

      float NdotL = dot(Normal, L);

      if (NdotL > 0)
      {
        float3 R = reflect(-L, Normal);

        resultColor += attenuation * LightSourcesPool[i].Color * diffuseColor * NdotL;

        float RdotV = dot(R, V);
        resultColor += attenuation * LightSourcesPool[i].Color * specularColor * pow(max(0.0f, RdotV), material.Ph);
      }
    }
    else if (LightSourcesPool[i].LightSourceType == LIGHT_SOURCE_TYPE_SPOT)
    {
      float3 LightPos = float3(0, 0, 0);
      float4x4 TransformMatrix = ObjectTransformData[LightSourcesPool[i].ObjectTransformIndex].transform;
      LightPos = mul(TransformMatrix, float4(LightPos, 1.0)).xyz;

      float3 LightDir = float3(0, -1, 0);
      TransformMatrix[0][3] = 0;
      TransformMatrix[1][3] = 0;
      TransformMatrix[2][3] = 0;
      LightDir = mul(TransformMatrix, float4(LightDir, 1.0)).xyz;
      LightDir = normalize(LightDir);

      float3 L = LightPos - Position;
      float d = length(L);
      L = normalize(L);

      float cosTheta = dot(-L, LightDir);
      float cosInnerCone = cos(LightSourcesPool[i].AngleInnerCone);
      float cosOuterCone = cos(LightSourcesPool[i].AngleOuterCone);


      if (cosTheta < cosOuterCone)
      {
        continue;
      }

      float attenuation = 1.0 / (
        LightSourcesPool[i].ConstantAttenuation +
        LightSourcesPool[i].LinearAttenuation * d +
        LightSourcesPool[i].QuadricAttenuation * d * d);

      if (cosTheta < cosInnerCone)
        attenuation *= 1.0 - (cosInnerCone - cosTheta) / (cosInnerCone - cosOuterCone);

      float NdotL = dot(Normal, L);

      if (NdotL > 0)
      {
        float3 R = reflect(-L, Normal);

        resultColor += attenuation * LightSourcesPool[i].Color * diffuseColor * NdotL;

        float RdotV = dot(R, V);
        resultColor += attenuation * LightSourcesPool[i].Color * specularColor * pow(max(0.0f, RdotV), material.Ph);
      }
    }
  }

  return float4(resultColor, alpha);
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

