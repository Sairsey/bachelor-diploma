#pragma once
#include "../unit_base.h"

class unit_shooter_enviroment : public gdr::unit_base
{
private:
  std::vector<gdr::gdr_index> RenderObjects;
  std::vector<gdr::gdr_index> StaticObjects;
  std::vector<gdr::gdr_index> DynamicObjects;
public:
  void Initialize(void)
  {
    // add plane
    RenderObjects.push_back(Engine->ObjectSystem->CreateObjectFromFile("bin/models/shooter_enviroment/static_env.glb"));
    StaticObjects = Engine->NewStaticMeshAssimp(gdr::physic_material(), "bin/models/shooter_enviroment/static_env.glb");

    // add boxes
    RenderObjects.push_back(Engine->ObjectSystem->CreateObjectFromFile("bin/models/shooter_enviroment/dynamic_env.glb"));
    DynamicObjects = Engine->NewDynamicMeshAssimp(gdr::physic_material(), "bin/models/shooter_enviroment/dynamic_env.glb");
    /*
    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "skybox Init");
    // Add skybox
    Engine->GlobalsSystem->CPUData.SkyboxCubemapIndex = Engine->CubeTexturesSystem->Load(
      "bin/cubemaps/Night/cubemap/px.hdr",
      "bin/cubemaps/Night/cubemap/nx.hdr",
      "bin/cubemaps/Night/cubemap/py.hdr",
      "bin/cubemaps/Night/cubemap/ny.hdr",
      "bin/cubemaps/Night/cubemap/pz.hdr",
      "bin/cubemaps/Night/cubemap/nz.hdr");

    Engine->GlobalsSystem->CPUData.IrradienceCubemapIndex = Engine->CubeTexturesSystem->Load(
      "bin/cubemaps/Night/irradiance/px.hdr",
      "bin/cubemaps/Night/irradiance/nx.hdr",
      "bin/cubemaps/Night/irradiance/py.hdr",
      "bin/cubemaps/Night/irradiance/ny.hdr",
      "bin/cubemaps/Night/irradiance/pz.hdr",
      "bin/cubemaps/Night/irradiance/nz.hdr");

    Engine->GlobalsSystem->CPUData.PrefilteredCubemapIndex = Engine->CubeTexturesSystem->LoadMips(
      "bin/cubemaps/Night/prefiltered",
      5);

    Engine->GlobalsSystem->CPUData.BRDFLUTIndex = Engine->TexturesSystem->Load("bin/cubemaps/Night/brdf.hdr");

    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
    */
  }

  void Response(void)
  {
    // Update boxes positions
    for (int i = 0; i < DynamicObjects.size(); i++)
    {
      gdr::gdr_node *Node = Engine->ObjectSystem->NodesPool[RenderObjects[1]].Find(Engine->GetPhysObject(DynamicObjects[i]).Name);
      if (Node != nullptr)
        Node->GetTransformEditable() = Engine->GetPhysObject(DynamicObjects[i]).GetTransform();
    }
  }

  std::string GetName(void)
  {
    return "unit_shooter_enviroment";
  }

  ~unit_shooter_enviroment(void)
  {
  }
};