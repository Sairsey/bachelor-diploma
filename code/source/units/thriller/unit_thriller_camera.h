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
    SplineKeypoints.push_back({-24.5959, -0.456267, 79.5904 });
    SplineKeypoints.push_back({-24.5799, -0.299011, 69.5922 });
    SplineKeypoints.push_back({-24.5555, -0.639248, 59.594 });
    SplineKeypoints.push_back({-24.5452, 1.30661, 49.7838 });
    SplineKeypoints.push_back({-25.2426, 0.246208, 39.8745 });
    SplineKeypoints.push_back({-27.4964, 0.0690202, 30.154 });
    SplineKeypoints.push_back({-31.0854, 0.474295, 20.8312 });
    SplineKeypoints.push_back({-34.8145, 0.717434, 11.5317 });
    SplineKeypoints.push_back({-36.2992, 1.02467, 1.77383 });
    SplineKeypoints.push_back({-32.3197, 1.07351, -5.40675 });
    SplineKeypoints.push_back({-22.8372, 0.762794, -6.44475 });
    SplineKeypoints.push_back({-15.4306, 0.473679, 0.155503 });
    SplineKeypoints.push_back({-8.94274, 0.223731, 7.77782 });
    SplineKeypoints.push_back({-1.03935, 0.197968, 13.8368 });
    SplineKeypoints.push_back({8.59253, 0.0145473, 16.464 });
    SplineKeypoints.push_back({18.5559, -0.237351, 17.4505 });
    SplineKeypoints.push_back({28.5767, 0.0396785, 17.6709 });
    SplineKeypoints.push_back({38.5787, 0.00712724, 17.839 });
    SplineKeypoints.push_back({48.576, 0.702479, 17.9254 });
    SplineKeypoints.push_back({58.5378, 1.09164, 17.8547 });
    SplineKeypoints.push_back({68.5332, 0.8, 17.643 });
    SplineKeypoints.push_back({78.4792, 1.68156, 16.9975 });
    SplineKeypoints.push_back({88.4708, 2.69596, 16.7508 });
    SplineKeypoints.push_back({98.2207, 3.65943, 18.4311 });
    SplineKeypoints.push_back({105.322, 3.94368, 25.2787 });
    SplineKeypoints.push_back({106.621, 3.44467, 34.941 });
    SplineKeypoints.push_back({106.688, 3.10195, 44.87 });
    SplineKeypoints.push_back({110.649, 3.09551, 54.0657 });
    SplineKeypoints.push_back({113.611, 3.35042, 63.6074 });
    SplineKeypoints.push_back({112.92, 4.60877, 73.376 });
    SplineKeypoints.push_back({104.906, 4.4802, 78.2791 });
    SplineKeypoints.push_back({95.0316, 4.13426, 76.7263 });
    SplineKeypoints.push_back({88.2993, 3.60353, 69.6572 });
    SplineKeypoints.push_back({83.6153, 3.51487, 60.7906 });
    SplineKeypoints.push_back({77.208, 3.335, 53.1843 });
    SplineKeypoints.push_back({68.0265, 2.62023, 49.5257 });
    SplineKeypoints.push_back({58.1628, 3.88841, 48.7364 });
    SplineKeypoints.push_back({48.18, 3.50935, 48.2588 });
    SplineKeypoints.push_back({38.2624, 2.05491, 48.2159 });
    SplineKeypoints.push_back({28.3745, 0.495043, 48.4289 });
    SplineKeypoints.push_back({18.4058, -0.40966, 48.7499 });
    SplineKeypoints.push_back({8.40664, -0.831586, 48.9412 });
    SplineKeypoints.push_back({-1.58057, -1.39252, 49.0172 });
    SplineKeypoints.push_back({-11.5671, -2.05468, 48.9609 });
    SplineKeypoints.push_back({-21.5212, -3.09541, 49.0071 });
    SplineKeypoints.push_back({-31.511, -3.86254, 48.9412 });
    SplineKeypoints.push_back({-41.4828, -4.78729, 49.083 });
    SplineKeypoints.push_back({-50.7321, -4.7643, 52.3812 });
    SplineKeypoints.push_back({-56.4272, -1.32331, 60.3767 });
    SplineKeypoints.push_back({-57.1851, -0.59387, 70.304 });
    SplineKeypoints.push_back({-56.4303, -1.57245, 80.2722 });
    SplineKeypoints.push_back({-54.9507, -3.39349, 90.1236 });
    SplineKeypoints.push_back({-52.724, -4.64472, 99.8532 });
    SplineKeypoints.push_back({-47.5172, -3.91068, 108.211 });
    SplineKeypoints.push_back({-38.8193, -3.00786, 112.925 });
    SplineKeypoints.push_back({-30.744, -1.94007, 110.224 });
    SplineKeypoints.push_back({-25.9996, -0.690897, 101.542 });
    SplineKeypoints.push_back({ -24.4807, - 0.186372, 91.6724});
  }

  mth::vec3f CatmulRomSpline(float t)
  {
    t -= int(t); // leave only frac part
    int p1 = t * SplineKeypoints.size();
    int p0 = (p1 - 1 + SplineKeypoints.size()) % SplineKeypoints.size();
    int p2 = (p1 + 1 + SplineKeypoints.size()) % SplineKeypoints.size();
    int p3 = (p1 + 2 + SplineKeypoints.size()) % SplineKeypoints.size();

    float t0 = 1.0 * p1 / (SplineKeypoints.size());
    float dt = 1.0 / (SplineKeypoints.size());
    
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
    mth::vec3f Pos2 = CatmulRomSpline((time + 0.1) / Duration);
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