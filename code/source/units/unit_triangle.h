#pragma once
#include "unit_base.h"

class unit_triangle : public gdr::unit_base
{
private:
  gdr::gdr_index Triangle;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_triangle";
  }

  ~unit_triangle(void)
  {
  }
};