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


    /* Terminate function
     * ARGUMENTS: None
     * RETURNS: None
     */
    void Term(void);

    // Default destructor
    ~render();
  };
}