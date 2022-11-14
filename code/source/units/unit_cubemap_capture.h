#pragma once
#include "unit_base.h"

class unit_cubemap_capture : public gdr::unit_base
{
private:
  int plane_number;
  std::string directory;
  mth::vec3f pos;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_cubemap_capture";
  }

  ~unit_cubemap_capture(void)
  {
  }
};