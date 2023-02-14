#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Pass representation class */
  class base_pass
  {
    protected:
      render *Render;
    public:

    // to track each pass time
    device_time_query DeviceTimeCounter;

    /* Default constructor */
    base_pass()
    {
    }

    /* Function to fill Render pointer*/
    void SetRender(render* Rnd)
    {
      Render = Rnd;
    }

    /* Function to Initialize every PSO/InputLayout/Shaders we need */
    virtual void Initialize(void) { return; };

    /* Function to get name */
    virtual std::string GetName(void) { return "base_pass"; };

    /* Function to call Direct draw shader */
    virtual void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) {};

    /* Function to call Indirect draw shader */
    virtual void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) { CallDirectDraw(currentCommandList); };

    /* Virtual Destructor */
    virtual ~base_pass()
    {
    }
  };
}