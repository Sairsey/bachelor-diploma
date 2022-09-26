#pragma once
#include "unit_base.h"

class unit_pbr_spheres : public gdr::unit_base
{
private:
  std::vector<gdr::gdr_object> Spheres;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_pbr_spheres";
  }

  ~unit_pbr_spheres(void)
  {
  }
};