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
  }

  void Response(void)
  {
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