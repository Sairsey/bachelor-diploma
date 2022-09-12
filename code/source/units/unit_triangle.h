#pragma once
#include "unit_base.h"

class unit_triangle : public gdr::unit_base
{
private:
  gdr::gdr_object Triangle;
public:
  void Initialize(void);

  void Response(void);

  ~unit_triangle(void)
  {
  }
};