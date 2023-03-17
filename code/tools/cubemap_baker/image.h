#pragma once
#include <string>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include "def.h"

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

  float_image()
  {
  }
  
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
    Direction = Direction.Normalize();
    mth::vec2f uv;
    // vector (0, 0, -1) should hit right in center
    // uv[0] = 0.5 <=> atan2f()/2pi = 0.5 <=> atan2f = pi

    // vector (-1, 0, 0) should be less then 0.5
    // uv[0] < 0.5 <=? atan2f()/2pi < 0.5 <=> atan2f() < pi <=> atan() < pi

    // (0, 1, 0) -> uv[1] = 0
    // (0, -1, 0) -> uv[1] = 1
    // (*, 0, *) -> uv[1] = 0.5

    uv[0] = atan2f(-Direction.X, Direction.Z) / (2 * MTH_PI);
    uv[1] = asinf(Direction.Y) / (MTH_PI / 2); // from -1 to 1
    uv[1] = 1.0f - (uv[1] + 1.0f) / 2.0f;

    if (uv[0] < 0)
        uv[0] += 1;

    uv[0] = max(0, min(1, uv[0]));
    uv[1] = max(0, min(1, uv[1]));

    uv[0] *= (W - 1);
    uv[1] *= (H - 1);

    int up_u = (int)ceil(uv[0]);
    int down_u = (int)floor(uv[0]);
    int up_v = (int)ceil(uv[1]);
    int down_v = (int)floor(uv[1]);
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
    x = max(0, min(x, W - 1));
    y = max(0, min(y, H - 1));
    return mth::vec3f(data[y * W * components + x * components], data[y * W * components + x * components + 1], data[y * W * components + x * components + 2]);
  }

  ~float_image()
  {
  }

};

class float_cube_image
{
public:
  // px, py, pz, nx, ny, nz
  float_image data[6];
  
  float_cube_image(std::string dir)
  {
    data[0] = float_image(dir + "/px.hdr");
    data[1] = float_image(dir + "/nx.hdr");
    data[2] = float_image(dir + "/py.hdr");
    data[3] = float_image(dir + "/ny.hdr");
    data[4] = float_image(dir + "/pz.hdr");
    data[5] = float_image(dir + "/nz.hdr");
  }

  mth::vec3f SampleCube(mth::vec3f Direction)
  {
    Direction.Normalize();

    float absX = fabs(Direction.X);
    float absY = fabs(Direction.Y);
    float absZ = fabs(Direction.Z);

    int isXPositive = Direction.X > 0 ? 1 : 0;
    int isYPositive = Direction.Y > 0 ? 1 : 0;
    int isZPositive = Direction.Z > 0 ? 1 : 0;

    float maxAxis, uc, vc;

    int index = -1;

    // POSITIVE X
    if (isXPositive && absX >= absY && absX >= absZ) {
      // u (0 to 1) goes from z to -z
      // v (0 to 1) goes from -y to +y
      maxAxis = absX;
      uc = Direction.Z;
      vc = -Direction.Y;
      index = 0;
    }
    // NEGATIVE X
    if (!isXPositive && absX >= absY && absX >= absZ) {
      // u (0 to 1) goes from -z to +z
      // v (0 to 1) goes from -y to +y
      maxAxis = absX;
      uc = -Direction.Z;
      vc = -Direction.Y;
      index = 1;
    }
    // POSITIVE Y
    if (isYPositive && absY >= absX && absY >= absZ) {
      // u (0 to 1) goes from -x to +x
      // v (0 to 1) goes from +z to -z
      maxAxis = absY;
      uc = Direction.X;
      vc = -Direction.Z;
      index = 2;
    }
    // NEGATIVE Y
    if (!isYPositive && absY >= absX && absY >= absZ) {
      // u (0 to 1) goes from -x to +x
      // v (0 to 1) goes from -z to +z
      maxAxis = absY;
      uc = Direction.X;
      vc = Direction.Z;
      index = 3;
    }
    // POSITIVE Z
    if (isZPositive && absZ >= absX && absZ >= absY) {
      // u (0 to 1) goes from -x to +x
      // v (0 to 1) goes from -y to +y
      maxAxis = absZ;
      uc = -Direction.X;
      vc = -Direction.Y;
      index = 4;
    }
    // NEGATIVE Z
    if (!isZPositive && absZ >= absX && absZ >= absY) {
      // u (0 to 1) goes from +x to -x
      // v (0 to 1) goes from -y to +y
      maxAxis = absZ;
      uc = Direction.X;
      vc = -Direction.Y;
      index = 5;
    }

    mth::vec2f uv;
    // Convert range from -1 to 1 to 0 to 1
    uv[0] = 0.5f * (uc / maxAxis + 1.0f);
    uv[1] = 0.5f * (vc / maxAxis + 1.0f);

    while (uv[1] < 0)
      uv[1] += 1;
    while (uv[0] < 0)
      uv[0] += 1;
    while (uv[1] > 1)
      uv[1] -= 1;
    while (uv[0] > 1)
      uv[0] -= 1;

    uv[1] = 1.0f - uv[1];

    uv[0] *= (data[index].W - 1);
    uv[1] *= (data[index].H - 1);

    int up_u = (int)ceil(uv[0]);
    int down_u = (int)floor(uv[0]);
    int up_v = (int)ceil(uv[1]);
    int down_v = (int)floor(uv[1]);
    float a1 = uv[1] - down_v;
    float a2 = uv[0] - down_u;

    mth::vec3f c3 =
      data[index].GetPixel(down_u, down_v) * ((1 - a2) * (1 - a1)) +
      data[index].GetPixel(down_u, up_v) * ((1 - a2) * a1) +
      data[index].GetPixel(up_u, down_v) * (a2 * (1 - a1)) +
      data[index].GetPixel(up_u, up_v) * (a2 * a1);

    return c3;
  }

  ~float_cube_image()
  {
  }

};