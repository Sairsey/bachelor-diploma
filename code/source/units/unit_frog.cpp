#include "p_header.h"

#include "unit_frog.h"

void unit_frog::Initialize(void)
{
  ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_frog Init");
    Frog = Engine->ObjectSystem->CreateObjectsFromFile("bin/models/crazy_frog/crazy_frog.obj")[0];
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
}

void unit_frog::Response(void)
{
  float radius = 10;
  mth::matr translation = mth::matr::Translate(mth::vec3f(radius, 0, 0));
  Engine->ObjectSystem->GetTransforms(Frog).transform = translation * mth::matr::RotateY(Engine->GetTime() * 21 * 6);
}