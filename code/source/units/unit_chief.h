#pragma once
#include "unit_base.h"

class unit_chief : public gdr::unit_base
{
private:
  gdr::gdr_index Chief;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_chief";
  }

  ~unit_chief(void)
  {
  }
};