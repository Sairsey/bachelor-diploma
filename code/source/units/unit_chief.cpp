#include "p_header.h"

#include "unit_chief.h"

void unit_chief::Initialize(void)
{
  Chief = Engine->ObjectSystem->CreateObjectFromFile("bin/models/chief/chief.glb");
}

void unit_chief::Response(void)
{
  Engine->ObjectSystem->SetAnimationTime(Chief, Engine->GetTime() * 1000.0);

  float radius = 5;
  mth::matr translation = mth::matr::Translate(mth::vec3f(radius, 0, 0));
  Engine->ObjectSystem->NodesPool[Chief].GetTransformEditable() = translation * mth::matr::RotateY(Engine->GetTime() * 20);
}