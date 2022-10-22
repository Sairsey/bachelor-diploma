#pragma once
#include "def.h"

// Project namespace
namespace gdr
{
  /* Input representation class */
  class input_support
  {
  public:
    BYTE
      Keys[256],           /* Current key states */
      OldKeys[256],        /* Previous frame key states */
      KeysClick[256];      /* Click key flags */
    INT
      Mx, My,              /* Mouse cursor position */
      Mz,                  /* Mouse wheel state*/
      Mdx, Mdy, Mdz;       /* Delta values of mouse axes */
    
    /* Default constructor */
    input_support(VOID);
  
    VOID Response(HWND hWnd);
  
    void UpdateWheel(int newZ)
    {
      Mdz = newZ - Mz;
      Mz = newZ;
    }

    ~input_support();
  }; /* end of 'input' class */
}