#include "p_header.h"

#include "unit_city.h"

void unit_city::Initialize(void)
{
  ID3D12GraphicsCommandList* commandList;
  //Engine->GetDevice().BeginUploadCommandList(&commandList);
  //PROFILE_BEGIN(commandList, "unit_city Init");
  //City = Engine->ObjectSystem->CreateObjectFromFile("bin/models/Sponza/NewSponza_Curtains_glTF.gltf");
  City = Engine->ObjectSystem->CreateObjectFromFile("bin/models/mutant/mutant.glb");
  //PROFILE_END(commandList);
  //Engine->GetDevice().CloseUploadCommandList();
}

void unit_city::Response(void)
{
}