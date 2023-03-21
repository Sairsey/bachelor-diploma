#pragma once
#include "../unit_base.h"

class unit_model_splitted : public gdr::unit_base
{
private:
  std::vector<gdr_index> Parts;
public:
  void Initialize(void)
  {
    auto import_data = gdr::ImportSplittedModelFromAssimp("bin/models/bathroom/bathroom.glb");

    ID3D12GraphicsCommandList* commandList;
    Engine->GetDevice().BeginUploadCommandList(&commandList);
    PROFILE_BEGIN(commandList, "unit_model_loading Init");
    for (auto &q : import_data)
      Parts.push_back(Engine->ModelsManager->Add(q));
    PROFILE_END(commandList);
    Engine->GetDevice().CloseUploadCommandList();
  }

  void Response(void)
  {
  }

  std::string GetName(void)
  {
    return "unit_model_splitted";
  }

  ~unit_model_splitted(void)
  {
    for (int i = 0; i < Parts.size(); i++)
      Engine->ModelsManager->Remove(Parts[i]);
  }
};