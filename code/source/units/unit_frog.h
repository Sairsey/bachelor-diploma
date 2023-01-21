#pragma once
#include "unit_base.h"

class unit_frog : public gdr::unit_base
{
private:
  gdr_index Frog;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_frog";
  }

  ~unit_frog(void)
  {
  }
};