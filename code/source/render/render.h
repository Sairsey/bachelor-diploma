#pragma once
#include "def.h"


/* Project namespace */
namespace gdr
{
  class engine;
  /* Render representation class */
  class render
  {
    private:
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

    /* Terminate function
     * ARGUMENTS: None
     * RETURNS: None
     */
    void Term(void);

    // Default destructor
    ~render();
  };
}