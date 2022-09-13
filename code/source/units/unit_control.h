#pragma once
#include "unit_base.h"

class unit_control : public gdr::unit_base
{
public:
  void Initialize(void);

  void Response(void);

  ~unit_control(void)
  {
  }
};