#pragma once
#include "def.h"


/* Project namespace */
namespace gdr
{
  class engine;
  class base_pass;
  class geometry_support;
  class globals_support;
  class object_support;
  class transforms_support;

  struct render_runtime_params
  {
    bool IsIndirect = false; // Enables indirect draw
  };

  /* Render representation class */
  class render
  {
    private:
      // Create Depth-stencil buffer
      bool CreateDepthStencil(void);

      // Current Engine 
      engine *Engine;
      // Init flag
      bool IsInited;
      // Device
      device Device;
      // Viewport
      D3D12_VIEWPORT Viewport;
      // Scissor rect
      D3D12_RECT Rect;
      // All supported passes
      std::vector<base_pass *> Passes;

      // Depth-Stencil buffer support
      GPUResource DepthBuffer;
      ID3D12DescriptorHeap* DSVHeap;
    public:
      /* Default Contructor */
      render();

      /* Initialize function 
       * ARGUMENTS:
       *      - Engine to init
       *          (engine *) Eng
       * RETURNS: true if success, false otherwise
       */
      bool Init(engine *Eng);

      /* Draw frame function */
      void DrawFrame(void);

      /* Resize frame function
       * ARGUMENTS:
       *      - new frame width
       *          (UINT) w
       *      - new frame height
       *          (UINT) h
       * RETURNS: None
       */
      void Resize(UINT w, UINT h);

      /* Get Device function */
      device &GetDevice(void) {return Device;}

      /* Terminate function
       * ARGUMENTS: None
       * RETURNS: None
       */
      void Term(void);

      // Default destructor
      ~render();

      // Current params
      render_runtime_params Params;
      // Subsystems
      globals_support *GlobalsSystem; // Store camera info and other important stuff
      geometry_support *GeometrySystem; // support of geometry creation
      transforms_support* TransformsSystem; // support of object Transforms
      object_support *ObjectSystem; // Helper of every subsystem
  };
}