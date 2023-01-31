#pragma once
#include "../unit_base.h"

class unit_thriller_env : public gdr::unit_base
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
    PROFILE_BEGIN(commandList, "unit_thriller_env Init");
    SkyboxIndex = Engine->CubeTexturesSystem->Add(
      "bin/cubemaps/ThrillerSky/cubemap/px.hdr",
      "bin/cubemaps/ThrillerSky/cubemap/nx.hdr",
      "bin/cubemaps/ThrillerSky/cubemap/py.hdr",
      "bin/cubemaps/ThrillerSky/cubemap/ny.hdr",
      "bin/cubemaps/ThrillerSky/cubemap/pz.hdr",
      "bin/cubemaps/ThrillerSky/cubemap/nz.hdr");
    BRDFLuTIndex = Engine->TexturesSystem->Add("bin/cubemaps/ThrillerSky/brdf.hdr");

    PrefilteredColorIndex = Engine->CubeTexturesSystem->Add("bin/cubemaps/ThrillerSky/prefiltered", 5);
    IrradianceIndex = Engine->CubeTexturesSystem->Add(
        "bin/cubemaps/ThrillerSky/irradiance/px.hdr",
        "bin/cubemaps/ThrillerSky/irradiance/nx.hdr",
        "bin/cubemaps/ThrillerSky/irradiance/py.hdr",
        "bin/cubemaps/ThrillerSky/irradiance/ny.hdr",
        "bin/cubemaps/ThrillerSky/irradiance/pz.hdr",
        "bin/cubemaps/ThrillerSky/irradiance/nz.hdr");

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
    return "unit_thriller_env";
  }

  ~unit_thriller_env(void)
  {
    Engine->CubeTexturesSystem->Remove(SkyboxIndex);
    Engine->CubeTexturesSystem->Remove(PrefilteredColorIndex);
    Engine->CubeTexturesSystem->Remove(IrradianceIndex);
    Engine->TexturesSystem->Remove(BRDFLuTIndex);
  }
};