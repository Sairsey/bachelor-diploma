#pragma once
#include "unit_base.h"

class unit_stats : public gdr::unit_base
{
private:

  // Transform tool
  bool TransformToolActive;
  int CurrentTransformToShow;
  
  // Material tool
  bool MaterialToolActive;
  int CurrentMaterialToShow;

public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_stats";
  }

  ~unit_stats(void)
  {
  }
};