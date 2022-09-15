#pragma once
#include "unit_base.h"

class unit_bread : public gdr::unit_base
{
private:
  std::vector<gdr::gdr_object> Bread;
public:
  void Initialize(void);

  void Response(void);

  ~unit_bread(void)
  {
  }
};