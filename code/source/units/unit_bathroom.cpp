#include "p_header.h"

#include "unit_bathroom.h"

void unit_bathroom::Initialize(void)
{
  Bathroom = Engine->ObjectSystem->CreateObjectFromFile("bin/models/bathroom/bathroom.glb");
}

void unit_bathroom::Response(void)
{
  Engine->ObjectSystem->NodesPool[Bathroom].GetTransformEditable() = mth::matr::Scale(15 / 2.0) * mth::matr::RotateY(-90-30) * mth::matr::Translate({20 / 2.0, -0.2, 0});
}