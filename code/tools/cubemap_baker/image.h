#pragma once
#include <string>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include "utils/math/mth.h"

struct pixel
{
  float R, G, B, A;
};

class float_image
{
  public:
    std::vector<float> data;
    int W = 0;
    int H = 0;
    int components = 0;

  float_image(std::string file_path)
  {    
    float *tmp_data = stbi_loadf(file_path.c_str(), &W, &H, &components, 0);
    data.resize(W * H * components);
    memcpy(&data[0], tmp_data, sizeof(float) * W * H * components);
    stbi_image_free(tmp_data);
  }

  float_image(int nW, int nH, int nComponents = 3) : W(nW), H(nH), components(nComponents)
  {
    data.resize(W * H * components);
  }

  void Resize(int newW, int newH)
  {
    float *new_data = (float*)malloc(newW * newH * components * sizeof(float));;
    stbir_resize_float(&data[0], W, H, 0, new_data, newW, newH, 0, components);
    data.resize(newW * newH * components);
    memcpy(&data[0], new_data, sizeof(float) * newW * newH * components);
    stbi_image_free(new_data);
    W = newW;
    H = newH;
  }

  void Save(std::string file_path)
  {
    stbi_write_hdr(file_path.c_str(), W, H, components, &data[0]);
  }

  mth::vec3f SampleHDR(mth::vec3f Direction)
  {
    Direction.Normalize();
    mth::vec2f uv;
    uv[0] = atan2f(Direction.Z, Direction.X) * 0.1591f;
    uv[1] = asinf(Direction.Y) * 0.3183f;
    uv[1] = 0.5f - uv[1];
    if (uv[1] < 0)
      uv[1] += 1;
    if (uv[0] < 0)
      uv[0] += 1;
    uv[0] *= (W - 1);
    uv[1] *= (H - 1);

    int up_u = ceil(uv[0]);
    int down_u = floor(uv[0]);
    int up_v = ceil(uv[1]);
    int down_v = floor(uv[1]);
    float a1 = uv[1] - down_v;
    float a2 = uv[0] - down_u;

    mth::vec3f c3 = 
      GetPixel(down_u, down_v) * ((1 - a2) * (1 - a1)) +
      GetPixel(down_u, up_v) * ((1 - a2) * a1) +
      GetPixel(up_u, down_v) * (a2 * (1 - a1)) +
      GetPixel(up_u, up_v) * (a2 * a1);

    return c3;
  }

  void PutPixel(mth::vec3f Pixel, int x, int y)
  {    
    if (components >= 1)
      data[ y * W * components + x * components + 0] = Pixel[0];
    if (components >= 2)
      data[y * W * components + x * components + 1] = Pixel[1];
    if (components >= 3)
      data[y * W * components + x * components + 2] = Pixel[2];
    if (components == 4)
      data[y * W * components + x * components + 3] = Pixel[3];
  }

  mth::vec3f GetPixel(int x, int y)
  {
    return mth::vec3f(data[y * W * components + x * components], data[y * W * components + x * components + 1], data[y * W * components + x * components + 2]);
  }

  ~float_image()
  {
  }

};