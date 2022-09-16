#pragma once
#include "unit_base.h"

class unit_frog : public gdr::unit_base
{
private:
  gdr::gdr_object Frog;
public:
  void Initialize(void);

  void Response(void);

  ~unit_frog(void)
  {
  }
};