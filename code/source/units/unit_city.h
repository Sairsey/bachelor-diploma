#pragma once
#include "unit_base.h"

class unit_city : public gdr::unit_base
{
private:
  gdr::gdr_object Frog;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_city";
  }

  ~unit_city(void)
  {
  }
};