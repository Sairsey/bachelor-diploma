#pragma once
#include "../unit_base.h"

class unit_shooter_enviroment : public gdr::unit_base
{
private:
  std::vector<gdr_index> StaticModels;
  std::vector<gdr_index> Static;
  std::vector<gdr_index> DynamicModels;
  std::vector<gdr_index> Dynamic;
public:
  void Initialize(void)
  {
    auto static_geom = gdr::ImportSplittedModelFromAssimp("bin/models/shooter_enviroment/static_env.glb");
    auto dynamic_geom = gdr::ImportSplittedModelFromAssimp("bin/models/shooter_enviroment/dynamic_env.glb");
    
    // add statics
    for (auto &i : static_geom)
    {
        ID3D12GraphicsCommandList* commandList;
        Engine->GetDevice().BeginUploadCommandList(&commandList);
        StaticModels.push_back(Engine->ModelsManager->Add(i));
        Engine->GetDevice().CloseUploadCommandList();
        
        Static.push_back(Engine->PhysicsManager->AddStaticMesh(i));
    }

    // add dynamics
    for (auto& i : dynamic_geom)
    {
        ID3D12GraphicsCommandList* commandList;
        Engine->GetDevice().BeginUploadCommandList(&commandList);
        DynamicModels.push_back(Engine->ModelsManager->Add(i));
        Engine->GetDevice().CloseUploadCommandList();

        Dynamic.push_back(Engine->PhysicsManager->AddDynamicMesh(i));
    }
  }

  void Response(void)
  {
    // Update boxes positions
    for (int i = 0; i < DynamicModels.size(); i++)
    {
      gdr_index ModelIndex = DynamicModels[i];
      gdr_index TransformIndex = Engine->ModelsManager->Get(ModelIndex).Render.RootTransform;
      Engine->ObjectTransformsSystem->GetEditable(TransformIndex).Transform = Engine->PhysicsManager->Get(TransformIndex).GetTransform();
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