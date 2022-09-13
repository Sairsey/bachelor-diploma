#include "p_header.h"

#include "unit_bread.h"

void unit_bread::Initialize(void)
{
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "unit_bread Init");

  Bread = Engine->ObjectSystem->CreateObjectsFromFile("bin/models/Bread/Bread.obj")[0];

  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_bread::Response(void)
{
  Engine->ObjectSystem->GetTransforms(Bread).transform = mth::matr::RotateY(Engine->GlobalsSystem->CPUData.time * 10.0);
}