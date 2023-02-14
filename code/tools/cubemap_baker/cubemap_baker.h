#pragma once
#include "image.h"

/* ATTENTION:
 * This module uses cubemaps in right-handed mode (also called OpenGL style).
 * This Decidion were made, because in my opinion right-handed CS is more convinient (law of right hand for "Cross product")
 * Input texture is panorama which looks kinda like that
 * 
 * ------------------------------------
 * |\ +y  /  \ +y  /  \ +y  /  \ +y  /|
 * | \   /    \   /    \   /    \   / |
 * |  \ /      \ /      \ /      \ /  |
 * |+z |   -x   |   -z   |   +x   | +z|
 * |  / \      / \      / \      / \  |
 * | /   \    /   \    /   \    /   \ |
 * |/ -y  \  / -y  \  / -y  \  / -y  \|
 * |__________________________________|
 * 
 * And cubemap should be saved like this (OpenGL style)
 *          ----------
 *          |        |
 *          |  (top) |
 *          |   +y   |
 *          |        |
 * -------------------------------------
 * |        |        |        |        |
 * | (left) | (front)| (right)| (back) |
 * |   -x   |   -z   |    x   |    +z  |
 * |        |        |        |        |
 * -------------------------------------
 *          |        |
 *          |(bottom)|
 *          |   -y   |
 *          |        |
 *          ----------
 * 
 */

namespace cubemap_baker_utils
{
  float RadicalInverse_VdC(unsigned int bits)
  {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(float(bits) * 2.3283064365386963e-10); // / 0x100000000
  }

  mth::vec2f Hammersley(unsigned int i, unsigned int N)
  {
    return mth::vec2f(float(i) / float(N), RadicalInverse_VdC(i));
  }

  BOOL DirectoryExists(LPCTSTR szPath)
  {
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
      (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
  }

  mth::vec3f ImportanceSampleGGX(mth::vec2f Xi, mth::vec3f norm, float roughness)
  {
    float a = roughness * roughness;

    float phi = 2.0f * MTH_PI * Xi.X;
    float cosTheta = sqrt((1.0f - Xi.Y) / (1.0f + (a * a - 1.0f) * Xi.Y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    mth::vec3f H;
    H.X = cos(phi) * sinTheta;
    H.Y = sin(phi) * sinTheta;
    H.Z = cosTheta;

    // from tangent-space vector to world-space sample vector
    mth::vec3f up = abs(norm.Z) < 0.999 ? mth::vec3f(0.0, 0.0, 1.0) : mth::vec3f(1.0, 0.0, 0.0);
    mth::vec3f tangent = (norm cross up).Normalize();
    mth::vec3f bitangent = tangent cross norm;

    mth::vec3f sampleVec = tangent * H.X + bitangent * H.Y + norm * H.Z;
    return sampleVec.Normalize();
  }

  float DistributionGGX(mth::vec3f N, mth::vec3f H, float roughness)
  {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(N dot H, 0.0f);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = max(MTH_PI * denom * denom, 0.0001f);

    if (nom == 0 && denom == 0)
      return 1.0;

    return nom / denom;
  }

  float GeometrySchlickGGX(float NdotV, float roughness)
  {
    float a = roughness;
    float k = (a * a) / 2.0f;

    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
  }

  float GeometrySmith(mth::vec3f N, mth::vec3f V, mth::vec3f L, float roughness)
  {
    float NdotV = max((N dot V), 0.0f);
    float NdotL = max((N dot L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
  }
}

class cubemap_baker
{
  private:
    std::string HDRImagePath;
    std::string OutputPath;
    float_image HDR;
    std::vector<float_image> HDR_mips;

    const char *names[6] = {
      "/px.hdr",
      "/py.hdr",
      "/pz.hdr",
      "/nx.hdr",
      "/ny.hdr",
      "/nz.hdr"
    };

    mth::vec3f dirs[6] = {
      {1, 0, 0},
      {0, 1, 0},  // I do now know why
      {0, 0, 1},
      {-1, 0, 0},
      {0, -1, 0},  // I do now know why
      {0, 0, -1}
    };

    mth::vec3f rights[6] = {
      {0, 0, 1},
      {1, 0, 0},
      {-1, 0, 0},
      {0, 0, -1},
      {1, 0, 0},
      {1, 0, 0}
    };

    float_image GetCubeSide(mth::vec3f dir, mth::vec3f right, int W, int H)
    {
      float_image cube_side = float_image(W, H);
      mth::vec3f up = right cross dir;
      for (int i = 0; i < H; i++) 
        for (int j = 0; j < W; j++)
        {
          mth::vec3f pos = right * (1.0f * (j + 0.5f) / W - 0.5f) + up * (1.0f * ((H - i - 1) + 0.5f) / H - 0.5f) + dir * 0.5f;

          cube_side.PutPixel(HDR.SampleHDR(pos), j, i);
        }
      return cube_side;
    }

    float_image GetPrefilteredSide(mth::vec3f dir, mth::vec3f right, int W, int H, float Roughness, int SampleCount)
    {
      float_image cube_side = float_image(W, H);
      mth::vec3f up = right cross dir;
      for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
        {
          mth::vec3f norm;
          norm = right * (1.0f * (j + 0.5f) / W - 0.5f) + up * (1.0f * ((H - i - 1) + 0.5f) / H - 0.5f) + dir * 0.5f;
          norm.Normalize();
          mth::vec3f view = norm;

          mth::vec3f prefilteredColor;
          float totalWeight = 0;

          for (int k = 0; k < SampleCount; k++)
          {
            mth::vec2f Xi = cubemap_baker_utils::Hammersley(k, SampleCount);
            mth::vec3f H = cubemap_baker_utils::ImportanceSampleGGX(Xi, norm, Roughness);
            mth::vec3f L = (H * 2.0 * (view dot H) - view).Normalized();

            float ndotl = max((norm dot L), 0.0f);
            float ndoth = max((norm dot H), 0.0f);
            float hdotv = max((H dot view), 0.0f);

            float D = cubemap_baker_utils::DistributionGGX(norm, H, Roughness);
            float pdf = (D * ndoth / (4.0f * hdotv)) + 0.0001f;

            float resolution = 512.0f; // resolution of source cubemap (per face)
            float saTexel = 4.0f * MTH_PI / (6.0f * resolution * resolution);
            float saSample = 1.0f / (float(SampleCount) * pdf + 0.0001f);

            float mipLevel = Roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);

            int down_mip = (int)floor(mipLevel);
            int up_mip = (int)ceil(mipLevel);
            float a = mipLevel - down_mip;

            if (ndotl > 0.0)
            {
              prefilteredColor +=  
                (HDR_mips[down_mip].SampleHDR(L) * (1 - a) + 
                HDR_mips[up_mip].SampleHDR(L) * (a)) * ndotl;
              totalWeight += ndotl;
            }
          }

          cube_side.PutPixel(prefilteredColor / totalWeight, j, i);
        }
      return cube_side;
    }

    float_image GetPreintegratedBRDFSide(int W, int H, int SampleCount)
    {
      float_image cube_side = float_image(W, H);
      for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
        {
          mth::vec2f value;
          float NdotV = 1.0f * (j + 0.5f) / W;
          float roughness = 1.0f * (i + 0.5f) / H;

          mth::vec3f prefilteredColor;
          float totalWeight = 0;

          mth::vec3f V;
          V.X = sqrt(1.0f - NdotV * NdotV);
          V.Y = 0.0f;
          V.Z = NdotV;

          float A = 0.0;
          float B = 0.0;

          mth::vec3f N = mth::vec3f(0.0, 0.0, 1.0);

          for (int k = 0; k < SampleCount; k++)
          {
            mth::vec2f Xi = cubemap_baker_utils::Hammersley(k, SampleCount);
            mth::vec3f H = cubemap_baker_utils::ImportanceSampleGGX(Xi, N, roughness);
            mth::vec3f L = (H * 2.0 * (V dot H) - V).Normalized();

            float NdotL = max(L.Z, 0.0f);
            float NdotH = max(H.Z, 0.0f);
            float VdotH = max((H dot V), 0.0f);

            if (NdotL > 0.0f)
            {
              float G = cubemap_baker_utils::GeometrySmith(N, V, L, roughness);
              float G_Vis = (G * VdotH) / (NdotH * NdotV);
              float Fc = pow(1.0f - VdotH, 5.0f);

              A += (1.0f - Fc) * G_Vis;
              B += Fc * G_Vis;
            }
          }

          A /= float(SampleCount);
          B /= float(SampleCount);

          cube_side.PutPixel(mth::vec3f(A, B, 0), j, i);
        }
      return cube_side;
    }

    float_image GetIrradianceSide(mth::vec3f dir, mth::vec3f right, int W, int H, int N1, int N2)
    {
      float_image cube_side = float_image(W, H);
      mth::vec3f up = right cross dir;
      for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
        {
          mth::vec3f normal;
          normal = right * (1.0f * (j + 0.5f) / W - 0.5f) + up * (1.0f * ((H - i - 1) + 0.5f) / H - 0.5f) + dir * 0.5f;
          normal.Normalize();
          mth::vec3f normal_up = {0, 1, 0};
          mth::vec3f normal_right = normal cross normal_up;
          normal_up = normal_right cross normal;

          normal_right.Normalize();
          normal_up.Normalize();

          mth::vec3f irradience;

          for (int k = 0; k < N1; k++)
            for (int l = 0; l < N2; l++)
            {
              float phi = k * (2.0f * MTH_PI / N1);
              float theta = l * (MTH_PI / 2.0f / N2);
              float sint = sin(theta);
              float cost = cos(theta);
              float sinp = sin(phi);
              float cosp = cos(phi);
              mth::vec3f tangentSample({ sint * cosp, sint * sinp, cost });

              mth::vec3f sampleVec = normal_right * tangentSample[0] + normal_up * tangentSample[1] + normal * tangentSample[2];

              irradience += HDR.SampleHDR(sampleVec) * cost * sint;
            }

          cube_side.PutPixel(irradience * MTH_PI / (1.0f * N1 * N2), j, i);
        }
      return cube_side;
    }

  public:
    cubemap_baker(std::string img, std::string dir) : HDRImagePath(img), OutputPath(dir), HDR(img)
    {
    }

    void BakeCubemap(void)
    {
      if (!cubemap_baker_utils::DirectoryExists((OutputPath + "\\cubemap").c_str()))
        CreateDirectoryA((OutputPath + "\\cubemap").c_str(), 0);
      printf("Cubemap creation\n");
      for (int i = 0; i < 6; i++)
      {
        printf("Side %i\n", i);
        float_image side = GetCubeSide(dirs[i], rights[i], 512, 512);
        side.Save(OutputPath + "\\cubemap" + names[i]);
      }
    }

    void BakeIrradience(void)
    {
      if (!cubemap_baker_utils::DirectoryExists((OutputPath + "\\irradiance").c_str()))
        CreateDirectoryA((OutputPath + "\\irradiance").c_str(), 0);
      printf("Irradiance map creation\n");
      for (int i = 0; i < 6; i++)
      {
        printf("Side %i\n", i);
        float_image side = GetIrradianceSide(dirs[i], rights[i], 32, 32, 1024, 1024);
        side.Save(OutputPath + "\\irradiance" + names[i]);
      }
    }

    void BakePrefilteredColor(void)
    {
      // Build mip-maps for HDR texture
      if (HDR_mips.size() == 0)
      {
        int MipsAmount = 10;
        for (int i = 0; i < MipsAmount; i++)
        {
          HDR_mips.push_back(HDR);
          HDR_mips[HDR_mips.size() - 1].Resize(HDR.W / (int)pow(2, i), HDR.H / (int)pow(2, i));
        }
      }

      if (!cubemap_baker_utils::DirectoryExists((OutputPath + "\\prefiltered").c_str()))
        CreateDirectoryA((OutputPath + "\\prefiltered").c_str(), 0);
      printf("Prefiltered Color map creation\n");
      int RoughnessProbes = 4;

      for (int probe = RoughnessProbes; probe >= 0; probe--)
      //for (int probe = 0; probe <= RoughnessProbes; probe++)
      {
        printf("Roughness %f\n", 1.0 * probe / RoughnessProbes);
        if (!cubemap_baker_utils::DirectoryExists((OutputPath + "\\prefiltered\\" + std::to_string(probe)).c_str()))
          CreateDirectoryA((OutputPath + "\\prefiltered\\" + std::to_string(probe)).c_str(), 0);
        for (int i = 0; i < 6; i++)
        {
          printf("Side %i\n", i);
          float_image side = GetPrefilteredSide(dirs[i], rights[i], 128 / (int)pow(2, probe), 128 / (int)pow(2, probe), 1.0f * probe / RoughnessProbes, 1024);
          side.Save(OutputPath + "\\prefiltered\\" + std::to_string(probe) + names[i]);
        }
      }
    }

    void BakePreintegratedBRDF(void)
    {
      // Build mip-maps for HDR texture
      if (HDR_mips.size() == 0)
      {
        int MipsAmount = 10;
        for (int i = 0; i < MipsAmount; i++)
        {
          HDR_mips.push_back(HDR);
          HDR_mips[HDR_mips.size() - 1].Resize(HDR.W / (int)pow(2, i), HDR.H / (int)pow(2, i));
        }
      }

      printf("Preintegrated BRDF Color map creation\n");      
      float_image side = GetPreintegratedBRDFSide(512, 512, 1024);
      side.Save(OutputPath + "\\brdf.hdr");
    }

};