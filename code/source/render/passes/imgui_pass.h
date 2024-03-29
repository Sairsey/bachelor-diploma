#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class imgui_pass : public base_pass
  {
    // will draw plane
  private:
    // lambdas to execute on imgui_pass
    std::vector<std::function<void(void)>> lambdas_to_draw;

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
  public:
    /* Function to get name */
    std::string GetName(void) override
    {
      return "imgui_pass";
    };

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    void Initialize(void) override;

    void AddLambda(std::function<void(void)> func) {lambdas_to_draw.push_back(func); }

    /* Function to call Direct draw shader */
    void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Function to call Indirect draw shader */
    //void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

    /* Virtual Destructor */
    ~imgui_pass() override;
  };
}
