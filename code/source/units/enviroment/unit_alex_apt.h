#pragma once
#include "../unit_base.h"

class unit_alex_apt : public gdr::unit_base
{
private:
  gdr_index SkyboxIndex;
  gdr_index BRDFLuTIndex;
  gdr_index PrefilteredColorIndex;
  gdr_index IrradianceIndex;
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
    BRDFLuTIndex = Engine->TexturesSystem->Add("bin/cubemaps/alexApt/brdf.hdr");

    PrefilteredColorIndex = Engine->CubeTexturesSystem->Add("bin/cubemaps/alexApt/prefiltered", 5);
    IrradianceIndex = Engine->CubeTexturesSystem->Add(
        "bin/cubemaps/alexApt/irradiance/px.hdr",
        "bin/cubemaps/alexApt/irradiance/nx.hdr",
        "bin/cubemaps/alexApt/irradiance/py.hdr",
        "bin/cubemaps/alexApt/irradiance/ny.hdr",
        "bin/cubemaps/alexApt/irradiance/pz.hdr",
        "bin/cubemaps/alexApt/irradiance/nz.hdr");

    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }

  void Response(void)
  {
      Engine->EnviromentSystem->GetEditable().MaxReflectionLod = 5 - 1; // number of mips in PrefilteredColorIndex - 1
      Engine->EnviromentSystem->GetEditable().SkyboxIndex = SkyboxIndex;
      Engine->EnviromentSystem->GetEditable().BRDFLUTIndex = BRDFLuTIndex;
      Engine->EnviromentSystem->GetEditable().IrradianceCubemapIndex = IrradianceIndex;
      Engine->EnviromentSystem->GetEditable().PrefilteredCubemapIndex = PrefilteredColorIndex;
  }

  std::string GetName(void)
  {
    return "unit_alex_apt";
  }

  ~unit_alex_apt(void)
  {
  }
};