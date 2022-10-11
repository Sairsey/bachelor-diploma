#include "p_header.h"

#include "unit_bathroom.h"

void unit_bathroom::Initialize(void)
{
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "unit_bathroom Init");
  Bathroom = Engine->ObjectSystem->CreateObjectFromFile("bin/models/bathroom/bathroom.glb");
  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_bathroom::Response(void)
{
}