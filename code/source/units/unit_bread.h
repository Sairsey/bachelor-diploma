#pragma once
#include "unit_base.h"

class unit_bread : public gdr::unit_base
{
private:
  std::vector<gdr::gdr_index> Bread;
  std::vector<mth::matr4f> Translations;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_breads";
  }

  ~unit_bread(void)
  {
  }
};