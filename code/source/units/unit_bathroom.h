#pragma once
#include "unit_base.h"

class unit_bathroom : public gdr::unit_base
{
private:
  gdr::gdr_index Bathroom;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_bathroom";
  }

  ~unit_bathroom(void)
  {
  }
};