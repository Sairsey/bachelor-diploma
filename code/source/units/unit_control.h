#pragma once
#include "unit_base.h"

class unit_control : public gdr::unit_base
{
private:
  float CameraSpeed;
  float MinCameraSpeed;
  float MaxCameraSpeed;
  float CameraSpeedStep;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_control";
  }

  ~unit_control(void)
  {
  }
};