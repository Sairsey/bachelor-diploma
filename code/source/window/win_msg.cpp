#include "p_header.h"

/* WM_CREATE window message handle function.
 * ARGUMENTS:
 *   - structure with creation data:
 *       CREATESTRUCT *CS;
 * RETURNS:
 *   (BOOL) TRUE to continue creation window, FALSE to terminate.
 */
BOOL gdr::win::OnCreate( CREATESTRUCT *CS )
{
  return TRUE;
}

/* WM_DESTROY window message handle function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::win::OnDestroy( VOID )
{
  /* Call user level deinitialization */
  if (IsInit)
  {
    IsInit = FALSE;
    Close();
    KillTimer(hWnd, 30);
  }
  PostQuitMessage(0);
}

/* WM_SIZE window message handle function.
 * ARGUMENTS:
 *   - sizing flag (see SIZE_***, like SIZE_MAXIMIZED)
 *       UINT State;
 *   - new width and height of client area:
 *       INT W, H;
 * RETURNS: None.
 */
VOID gdr::win::OnSize( UINT State, INT W, INT H )
{
  Width = max(W, 8);
  Height = max(H, 8);
  /* Call user level change size notifivcation */
  if (IsInit)
    Resize();
}

/* WM_ERASEBKGND window message handle function.
 * ARGUMENTS:
 *   - device context of client area:
 *       HDC hDC;
 * RETURNS:
 *   (BOOL) TRUE if background is erased, FALSE otherwise.
 */
BOOL gdr::win::OnEraseBkgnd( HDC hDC )
{
  /* Call user level erase background function */
  if (IsInit)
    Erase(hDC);
  return FALSE;
}

/* WM_PAINT window message handle function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::win::OnPaint( VOID )
{
  HDC hDC;
  PAINTSTRUCT ps;

  hDC = BeginPaint(hWnd, &ps);
  /* Call user level paint window content function */
  if (IsInit)
    Paint(hDC);
  EndPaint(hWnd, &ps); 
}

/* WM_ACTIVATE window message handle function.
 * ARGUMENTS:
 *   - reason (WA_CLICKACTIVE, WA_ACTIVE or WA_INACTIVE):
 *       UINT Reason;
 *   - handle of active window:
 *       HWND hWndActDeact;
 *   - minimized flag:
 *       BOOL IsMinimized;
 * RETURNS: None.
 */
VOID gdr::win::OnActivate( UINT Reason, HWND hWndActDeact, BOOL IsMinimized )
{
  IsActive = Reason == WA_CLICKACTIVE || Reason == WA_ACTIVE;

  /* Call user level activate handle function */
  if (IsInit)
    Activate(IsActive);
}

/* WM_TIMER window message handle function.
 * ARGUMENTS:
 *   - specified the timer identifier.
 *       INT Id;
 * RETURNS: None.
 */
VOID gdr::win::OnTimer( INT Id )
{
  if (Id == InitializationTimer)
  {
    IsInit = TRUE;
    Init();
    Resize();
    InvalidateRect(hWnd, NULL, TRUE);
    if (IsActive)
      Activate(TRUE);
    SetTimer(hWnd, 30, 1, NULL);
  }
  else
    /* Call user level timer handle function */
    if (IsInit)
      Timer();
  if (IsChanged)
  {
    IsChanged = FALSE;
    SetFullScreen();
  }
}

/* WM_*BUTTONDOWN window message handle function.
 * ARGUMENTS:
 *   - double click flag:
 *       BOOL IsDoubleClick;
 *   - mouse window position:
 *       INT X, Y;
 *   - mouse keys bits (see MK_*** bits constants):
 *       UINT Keys;
 * RETURNS: None.
 */
VOID gdr::win::OnButtonDown( BOOL IsDoubleClick, INT X, INT Y, UINT Keys )
{
  SetCapture(hWnd);
}

/* WM_*BUTTONUP window message handle function.
 * ARGUMENTS:
 *   - mouse window position:
 *       INT X, Y;
 *   - mouse keys bits (see MK_*** bits constants):
 *       UINT Keys;
 * RETURNS: None.
 */
VOID gdr::win::OnButtonUp( INT X, INT Y, UINT Keys )
{
  ReleaseCapture();
}

/* WM_MOUSEWHEEL window message handle function.
 * ARGUMENTS:
 *   - mouse window position:
 *       INT X, Y;
 *   - mouse wheel delta value (120 points per rotate):
 *       INT Z;
 *   - mouse keys bits (see MK_*** bits constants):
 *       UINT Keys;
 * RETURNS: None.
 */
VOID gdr::win::OnMouseWheel( INT X, INT Y, INT Z, UINT Keys )
{
  MouseWheel += Z;
}

