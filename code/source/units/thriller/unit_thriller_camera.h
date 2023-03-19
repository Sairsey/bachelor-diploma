#pragma once
#include "../unit_base.h"

#include <Windows.h>
#include <Mmsystem.h>

#pragma comment(lib, "winmm.lib")

class unit_thriller_camera : public gdr::unit_base
{
private:
  std::vector<mth::vec3f> SplineKeypoints;
  float Duration = 2 * 60 + 13; // seconds
  float Offset = 0;
public:
  void Initialize(void)
  {
    SplineKeypoints.push_back({-24.5959f, -0.456267f, 79.5904f });
    SplineKeypoints.push_back({-24.5799f, -0.299011f, 69.5922f });
    SplineKeypoints.push_back({-24.5555f, -0.639248f, 59.594f });
    SplineKeypoints.push_back({-24.5452f, 1.30661f, 49.7838f });
    SplineKeypoints.push_back({-25.2426f, 0.246208f, 39.8745f });
    SplineKeypoints.push_back({-27.4964f, 0.0690202f, 30.154f });
    SplineKeypoints.push_back({-31.0854f, 0.474295f, 20.8312f });
    SplineKeypoints.push_back({-34.8145f, 0.717434f, 11.5317f });
    SplineKeypoints.push_back({-36.2992f, 1.02467f, 1.77383f });
    SplineKeypoints.push_back({-32.3197f, 1.07351f, -5.40675f });
    SplineKeypoints.push_back({-22.8372f, 0.762794f, -6.44475f });
    SplineKeypoints.push_back({-15.4306f, 0.473679f, 0.155503f });
    SplineKeypoints.push_back({-8.94274f, 0.223731f, 7.77782f });
    SplineKeypoints.push_back({-1.03935f, 0.197968f, 13.8368f });
    SplineKeypoints.push_back({8.59253f, 0.0145473f, 16.464f });
    SplineKeypoints.push_back({18.5559f, -0.237351f, 17.4505f });
    SplineKeypoints.push_back({28.5767f, 0.0396785f, 17.6709f });
    SplineKeypoints.push_back({38.5787f, 0.00712724f, 17.839f });
    SplineKeypoints.push_back({48.576f, 0.702479f, 17.9254f });
    SplineKeypoints.push_back({58.5378f, 1.09164f, 17.8547f });
    SplineKeypoints.push_back({68.5332f, 0.8f, 17.643f });
    SplineKeypoints.push_back({78.4792f, 1.68156f, 16.9975f });
    SplineKeypoints.push_back({88.4708f, 2.69596f, 16.7508f });
    SplineKeypoints.push_back({98.2207f, 3.65943f, 18.4311f });
    SplineKeypoints.push_back({105.322f, 3.94368f, 25.2787f });
    SplineKeypoints.push_back({106.621f, 3.44467f, 34.941f });
    SplineKeypoints.push_back({106.688f, 3.10195f, 44.87f });
    SplineKeypoints.push_back({110.649f, 3.09551f, 54.0657f });
    SplineKeypoints.push_back({113.611f, 3.35042f, 63.6074f });
    SplineKeypoints.push_back({112.92f, 4.60877f, 73.376f });
    SplineKeypoints.push_back({104.906f, 4.4802f, 78.2791f });
    SplineKeypoints.push_back({95.0316f, 4.13426f, 76.7263f });
    SplineKeypoints.push_back({88.2993f, 3.60353f, 69.6572f });
    SplineKeypoints.push_back({83.6153f, 3.51487f, 60.7906f });
    SplineKeypoints.push_back({77.208f, 3.335f, 53.1843f });
    SplineKeypoints.push_back({68.0265f, 2.62023f, 49.5257f });
    SplineKeypoints.push_back({58.1628f, 3.88841f, 48.7364f });
    SplineKeypoints.push_back({48.18f, 3.50935f, 48.2588f });
    SplineKeypoints.push_back({38.2624f, 2.05491f, 48.2159f });
    SplineKeypoints.push_back({28.3745f, 0.495043f, 48.4289f });
    SplineKeypoints.push_back({18.4058f, -0.40966f, 48.7499f });
    SplineKeypoints.push_back({8.40664f, -0.831586f, 48.9412f });
    SplineKeypoints.push_back({-1.58057f, -1.39252f, 49.0172f });
    SplineKeypoints.push_back({-11.5671f, -2.05468f, 48.9609f });
    SplineKeypoints.push_back({-21.5212f, -3.09541f, 49.0071f });
    SplineKeypoints.push_back({-31.511f, -3.86254f, 48.9412f });
    SplineKeypoints.push_back({-41.4828f, -4.78729f, 49.083f });
    SplineKeypoints.push_back({-50.7321f, -4.7643f, 52.3812f });
    SplineKeypoints.push_back({-56.4272f, -1.32331f, 60.3767f });
    SplineKeypoints.push_back({-57.1851f, -0.59387f, 70.304f });
    SplineKeypoints.push_back({-56.4303f, -1.57245f, 80.2722f });
    SplineKeypoints.push_back({-54.9507f, -3.39349f, 90.1236f });
    SplineKeypoints.push_back({-52.724f, -4.64472f, 99.8532f });
    SplineKeypoints.push_back({-47.5172f, -3.91068f, 108.211f });
    SplineKeypoints.push_back({-38.8193f, -3.00786f, 112.925f });
    SplineKeypoints.push_back({-30.744f, -1.94007f, 110.224f });
    SplineKeypoints.push_back({-25.9996f, -0.690897f, 101.542f });
    SplineKeypoints.push_back({ -24.4807f, -0.186372f, 91.6724f});
  }

  mth::vec3f CatmulRomSpline(float t)
  {
    t -= int(t); // leave only frac part
    int p1 = (int)(t * SplineKeypoints.size());
    int p0 = (int)((p1 - 1 + SplineKeypoints.size()) % SplineKeypoints.size());
    int p2 = (int)((p1 + 1 + SplineKeypoints.size()) % SplineKeypoints.size());
    int p3 = (int)((p1 + 2 + SplineKeypoints.size()) % SplineKeypoints.size());

    float t0 = 1.0f * p1 / (SplineKeypoints.size());
    float dt = 1.0f / (SplineKeypoints.size());
    
    t = (t - t0) / dt;

    float t_2 = t * t;
    float t_3 = t * t * t;

    mth::vec3f result = 
      (SplineKeypoints[p1] * ( 2)                                                                                       )       +
      (SplineKeypoints[p0] * (-1)                              + SplineKeypoints[p2] * ( 1)                             ) * t   +
      (SplineKeypoints[p0] * ( 2) + SplineKeypoints[p1] * (-5) + SplineKeypoints[p2] * ( 4) + SplineKeypoints[p3] * (-1)) * t_2 +
      (SplineKeypoints[p0] * (-1) + SplineKeypoints[p1] * ( 3) + SplineKeypoints[p2] * (-3) + SplineKeypoints[p3] * ( 1)) * t_3;
    result *= 0.5f;
    return result;
  }

  void Response(void)
  {
    static bool IsFirst = true;
    if (IsFirst)
    {
      Offset = Engine->GetTime();
      PlaySound(TEXT("bin/sounds/driving.wav"), NULL, SND_FILENAME | SND_ASYNC);
      IsFirst = false;
      Engine->ToggleFullScreen();
    }

    if (Engine->Keys[VK_RBUTTON])
    {
      if (Engine->Mdx != 0)
      {
        Engine->PlayerCamera.RotateAroundLocY(-Engine->Mdx * 0.1f);
      }
      if (Engine->Mdy != 0)
      {
        Engine->PlayerCamera.RotateAroundLocRight(-Engine->Mdy * 0.1f);
      }
    }

    float time = Engine->GetTime() - Offset;

    mth::vec3f Pos = CatmulRomSpline(time / Duration);
    mth::vec3f Pos2 = CatmulRomSpline((time + 0.1f) / Duration);
    Engine->PlayerCamera.SetPos(Pos);
    Engine->PlayerCamera.SetDir((Pos2 - Pos).Normalized());
  }

  std::string GetName(void)
  {
    return "unit_thriller_camera";
  }

  ~unit_thriller_camera(void)
  {
  }
};