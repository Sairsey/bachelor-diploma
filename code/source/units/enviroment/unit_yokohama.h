#pragma once
#include "../unit_base.h"

class unit_yokohama : public gdr::unit_base
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
      "bin/cubemaps/yokohama/posx.jpg",
      "bin/cubemaps/yokohama/negx.jpg",
      "bin/cubemaps/yokohama/posy.jpg",
      "bin/cubemaps/yokohama/negy.jpg",
      "bin/cubemaps/yokohama/posz.jpg",
      "bin/cubemaps/yokohama/negz.jpg");
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }

  void Response(void)
  {
    Engine->Params.SkyboxIndex = SkyboxIndex;
  }

  std::string GetName(void)
  {
    return "unit_yokohama";
  }

  ~unit_yokohama(void)
  {
  }
};