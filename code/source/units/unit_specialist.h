#pragma once
#include "unit_base.h"

class unit_specialist : public gdr::unit_base
{
private:
  gdr::gdr_index Crash;
  gdr::gdr_index Crash2;
  gdr::gdr_index Crash3;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_specialist";
  }

  ~unit_specialist(void)
  {
  }
};