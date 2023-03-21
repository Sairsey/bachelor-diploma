#pragma once
#include "../unit_base.h"

class unit_triangle : public gdr::unit_base
{
private:
  gdr_index TriangleDrawCall;
  gdr_index TriangleTransform;
  gdr_index TriangleMaterial;
  gdr_index TriangleGeometry;
public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_triangle";
  }

  ~unit_triangle(void)
  {
    Engine->DrawCommandsSystem->Remove(TriangleDrawCall);
  }
};