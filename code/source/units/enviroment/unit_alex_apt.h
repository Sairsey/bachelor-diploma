#pragma once
#include "../unit_base.h"

class unit_alex_apt : public gdr::unit_base
{
private:
  gdr_index SkyboxIndex;
public:
  void Initialize(void)
  {
    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_alex_apartament Init");
    SkyboxIndex = Engine->CubeTexturesSystem->Add(
      "bin/cubemaps/alexApt/cubemap/px.hdr",
      "bin/cubemaps/alexApt/cubemap/nx.hdr",
      "bin/cubemaps/alexApt/cubemap/py.hdr",
      "bin/cubemaps/alexApt/cubemap/ny.hdr",
      "bin/cubemaps/alexApt/cubemap/pz.hdr",
      "bin/cubemaps/alexApt/cubemap/nz.hdr");
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }

  void Response(void)
  {
    Engine->Params.SkyboxIndex = SkyboxIndex;
  }

  std::string GetName(void)
  {
    return "alex_apt";
  }

  ~unit_alex_apt(void)
  {
  }
};