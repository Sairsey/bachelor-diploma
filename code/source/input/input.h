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
    input_support(VOID)
    {
    } /* End of 'input' function */
  
    VOID Response(HWND hWnd)
    {
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(hWnd, &pt);
      Mdx = pt.x - Mx;
      Mdy = pt.y - My;
      Mx = pt.x;
      My = pt.y;
  
      memcpy(OldKeys, Keys, 256);
      GetKeyboardState(Keys);
      for (INT i = 0; i < 256; i++)
      {
        Keys[i] >>= 7;
        KeysClick[i] = !OldKeys[i] && Keys[i];
      }
    } /* End of 'Response' function */
  
    void UpdateWheel(int newZ)
    {
      Mdz = newZ - Mz;
      Mz = newZ;
    }
  }; /* end of 'input' class */
}