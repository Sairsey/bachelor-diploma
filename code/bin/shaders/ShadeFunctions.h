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
    ambientColor = TexturesPool[material.KaMapIndex].Sample(LinearSampler, uv).xyz;

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
    float attenuation = 1.0;
    float3 L;

    if (LightSourcesPool[i].LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
    {
      float3 LightDir = float3(0, -1, 0);
      float4x4 TransformMatrix = ObjectTransformData[LightSourcesPool[i].ObjectTransformIndex].transform;
      TransformMatrix[0][3] = 0;
      TransformMatrix[1][3] = 0;
      TransformMatrix[2][3] = 0;
      LightDir = mul(TransformMatrix, float4(LightDir, 1.0)).xyz;
      LightDir = normalize(LightDir);
      L = -LightDir;
    }
    else if (LightSourcesPool[i].LightSourceType == LIGHT_SOURCE_TYPE_POINT)
    {
      float3 LightPos = float3(0, 0, 0);
      float4x4 TransformMatrix = ObjectTransformData[LightSourcesPool[i].ObjectTransformIndex].transform;
      LightPos = mul(TransformMatrix, float4(LightPos, 1.0)).xyz;
      
      L = LightPos - Position;
      float d = length(L);
      L = normalize(L);

      attenuation = 1.0 / (
        LightSourcesPool[i].ConstantAttenuation +
        LightSourcesPool[i].LinearAttenuation * d +
        LightSourcesPool[i].QuadricAttenuation * d * d);
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

      L = LightPos - Position;
      float d = length(L);
      L = normalize(L);

      float cosTheta = dot(-L, LightDir);
      float cosInnerCone = cos(LightSourcesPool[i].AngleInnerCone);
      float cosOuterCone = cos(LightSourcesPool[i].AngleOuterCone);

      if (cosTheta < cosOuterCone)
      {
        continue;
      }

      attenuation = 1.0 / (
        LightSourcesPool[i].ConstantAttenuation +
        LightSourcesPool[i].LinearAttenuation * d +
        LightSourcesPool[i].QuadricAttenuation * d * d);

      if (cosTheta < cosInnerCone)
        attenuation *= 1.0 - (cosInnerCone - cosTheta) / (cosInnerCone - cosOuterCone);
    }

    float NdotL = dot(Normal, L);

    if (NdotL > 0)
    {
      float3 R = reflect(-L, Normal);

      resultColor += attenuation * LightSourcesPool[i].Color * diffuseColor * NdotL;

      float RdotV = dot(R, V);
      resultColor += attenuation * LightSourcesPool[i].Color * specularColor * pow(max(0.0f, RdotV), material.Ph);
    }
  }

  return float4(resultColor, alpha);
}

#define PI 3.14159265358979323846

float GGX_PartialGeometry(float cosThetaN, float alpha) {
  float K = (alpha + 1) * (alpha + 1) / 8.0;  
  return cosThetaN / (cosThetaN * (1 - K) + K);
}

float GGX_Distribution(float cosThetaNH, float alpha) {
  float alpha2 = alpha * alpha;
  float NH_sqr = saturate(cosThetaNH * cosThetaNH);
  float den = NH_sqr * alpha2 + (1.0 - NH_sqr);
  return alpha2 / (PI * den * den);
}

float3 FresnelSchlick(float3 F0, float cosTheta) {
  return F0 + (1.0 - F0) * pow(1.0 - saturate(cosTheta), 5.0);
}

float4 ShadeCookTorrance(float3 Normal, float3 Position, float2 uv, ObjectMaterial material)
{
  float3 resultColor = float3(0, 0, 0);
  float alpha = 1.0;
  
  float roughness = material.Ks.g;
  if (material.KsMapIndex != -1)
    roughness = TexturesPool[material.KsMapIndex].Sample(LinearSampler, uv).g;

  float3 albedo = material.Kd;
  if (material.KdMapIndex != -1)
  {
    albedo = TexturesPool[material.KdMapIndex].Sample(LinearSampler, uv).xyz;
    alpha = TexturesPool[material.KdMapIndex].Sample(NearestSampler, uv).w;
  }

  float metallic = material.Ks.b;
  if (material.KsMapIndex != -1)
    metallic = TexturesPool[material.KsMapIndex].Sample(LinearSampler, uv).b;

  float3 V = globals.CameraPos - Position;
  V = normalize(V);

  resultColor = 0.0;

  for (uint i = 0; i < globals.LightsAmount; i++)
  {
    float attenuation = 1.0;
    float3 L;

    if (LightSourcesPool[i].LightSourceType == LIGHT_SOURCE_TYPE_DIRECTIONAL)
    {
      float3 LightDir = float3(0, -1, 0);
      float4x4 TransformMatrix = ObjectTransformData[LightSourcesPool[i].ObjectTransformIndex].transform;
      TransformMatrix[0][3] = 0;
      TransformMatrix[1][3] = 0;
      TransformMatrix[2][3] = 0;
      LightDir = mul(TransformMatrix, float4(LightDir, 1.0)).xyz;
      LightDir = normalize(LightDir);
      L = -LightDir;
    }
    else if (LightSourcesPool[i].LightSourceType == LIGHT_SOURCE_TYPE_POINT)
    {
      float3 LightPos = float3(0, 0, 0);
      float4x4 TransformMatrix = ObjectTransformData[LightSourcesPool[i].ObjectTransformIndex].transform;
      LightPos = mul(TransformMatrix, float4(LightPos, 1.0)).xyz;
      
      L = LightPos - Position;
      float d = length(L);
      L = normalize(L);

      attenuation = 1.0 / (
        LightSourcesPool[i].ConstantAttenuation +
        LightSourcesPool[i].LinearAttenuation * d +
        LightSourcesPool[i].QuadricAttenuation * d * d);
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

      L = LightPos - Position;
      float d = length(L);
      L = normalize(L);

      float cosTheta = dot(-L, LightDir);
      float cosInnerCone = cos(LightSourcesPool[i].AngleInnerCone);
      float cosOuterCone = cos(LightSourcesPool[i].AngleOuterCone);

      if (cosTheta < cosOuterCone)
      {
        continue;
      }

      attenuation = 1.0 / (
        LightSourcesPool[i].ConstantAttenuation +
        LightSourcesPool[i].LinearAttenuation * d +
        LightSourcesPool[i].QuadricAttenuation * d * d);

      if (cosTheta < cosInnerCone)
        attenuation *= 1.0 - (cosInnerCone - cosTheta) / (cosInnerCone - cosOuterCone);
    }

    float3 H = normalize(V + L);
    //precompute dots
    float NL = max(dot(Normal, L), 0.0);
    float NV = max(dot(Normal, V), 0.0);
    float NH = max(dot(Normal, H), 0.0);
    float HV = max(dot(H, V), 0.0);

    float3 F0 = 0.04;
    F0 = lerp(F0, albedo, metallic);
    
    //precompute roughness square
    float G = GGX_PartialGeometry(NV, roughness) * GGX_PartialGeometry(NL, roughness);
    float D = GGX_Distribution(NH, roughness);
    float3 F = FresnelSchlick(F0, HV);

    float3 Specular = G * D * F / (4.0 * (NV + 1e-5f));
    float3 KDiffuse = float3(1.0, 1.0, 1.0) - F;
    float3 Diffuse = albedo * (1.0 - metallic) * KDiffuse * NL / PI;

    resultColor += attenuation * LightSourcesPool[i].Color * (Diffuse + Specular);
  }
  
  // enviroment color
  if (globals.SkyboxCubemapIndex != -1)
  {
    // lets pretent we get only light from point Light Source by normal
    float3 L = Normal;

    float3 H = normalize(V + L);
    //precompute dots
    float NL = max(dot(Normal, L), 0.0);
    float NV = max(dot(Normal, V), 0.0);
    float NH = max(dot(Normal, H), 0.0);
    float HV = max(dot(H, V), 0.0);

    float3 F0 = 0.04;
    F0 = lerp(F0, albedo, metallic);

    //precompute roughness square
    float G = GGX_PartialGeometry(NV, roughness) * GGX_PartialGeometry(NL, roughness);
    float D = GGX_Distribution(NH, roughness);
    float3 F = FresnelSchlick(F0, HV);

    float3 Specular = G * D * F / (4.0 * (NV + 1e-5f));
    float3 KDiffuse = float3(1.0, 1.0, 1.0) - F;
    float3 Diffuse = albedo * (1.0 - metallic) * KDiffuse * NL / PI;
    float3 r = reflect(-V, Normal);

    resultColor += CubeTexturesPool[globals.SkyboxCubemapIndex].Sample(LinearSampler, r).rgb * Specular + Diffuse;
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
    case MATERIAL_SHADER_COOKTORRANCE:
      return ShadeCookTorrance(Normal, Position, uv, material);
      break;
    default:
      return float4(1, 0, 1, 1);
  }
}

