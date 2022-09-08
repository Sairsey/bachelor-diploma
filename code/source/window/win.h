#pragma once

#include <Windows.h>

#include "def.h"

/* Project namespace */
namespace gdr
{
  /* Window class name */
  const TCHAR MainWndClassName[] = L"GDR Project";

  /* Main window class declaration */
  class win
  {
  private:
    /* Initialization timer identifier */
    static const INT InitializationTimer = 303030;

    /* Save rectangle and window style for full screen mode */
    RECT SaveRect;
    DWORD SaveStyle;

    /* Keyboard adapter input char function.
     * ARGUMENTS:
     *   - inputed char:
     *       CHAR Char;
     * RETURNS: None.
     */
    VOID (* CharAdapter)( CHAR Char ) = NULL;

    /* Keyboard adapter keydown function.
     * ARGUMENTS:
     *   - keydown:
     *       CHAR Char;
     * RETURNS: None.
     */
    VOID (* KeydownAdapter)( CHAR Char ) = NULL;

    /* Enable/disable full screen window mode function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    VOID SetFullScreen( VOID );

  protected:
    /* Initialization flag */
    BOOL IsInit;

    /* Window data */
    HWND hWnd;                // Window handle
    HINSTANCE hInstance;      // Application instance handle
    INT Width, Height;        // Window size
    BOOL IsActive;            // Active flag
    BOOL IsFullScreen;        // Full screen flag
    BOOL IsChanged;           // Full screen change flag
    INT MouseWheel;           // Wheel relative rotate counter

  public:

    /* Default class constructor. */
    win( VOID );

    /* Class destructor */
    ~win( VOID );

    /* Windowed application running function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (WPARAM) Error level for operation system (0 for success).
     */
    WPARAM Run( VOID );

    /* Write text in header function.
     * ARGUMENTS: None.
     *    - string to write;
     *       std::string &Str;
     * RETURNS: None.
     */
    VOID WriteHeader( const std::string &Str );

        /* Set keyboard adapter input char function 
     * ARGUMENTS:
     *   - pointer on function
     *       VOID (* Adapter)( CHAR Char )
     * RETURNS: None.
     */
    VOID SetCharAdapter( VOID (* Adapter)( CHAR Char ) )
    {
      CharAdapter = Adapter;
    } /* End of 'SetCharAdapter' function */

    /* Set keyboard adapter keydown function 
     * ARGUMENTS:
     *   - pointer on function
     *       VOID (* Adapter)( CHAR Char )
     * RETURNS: None.
     */
    VOID SetKeydownAdapter( VOID (* Adapter)( CHAR Char ) )
    {
      KeydownAdapter = Adapter;
    } /* End of 'SetCharAdapter' function */

    /* Window message handle function (CALLBACK version).
     *   - window handle:
     *       HWND hWnd;
     *   - message identifier (see WM_***):
     *       UINT Msg;
     *   - message first parameter ('word' param):
     *       WPARAM wParam;
     *   - message second parameter ('long' param):
     *       LPARAM lParam;
     * RETURNS:
     *   (LRESULT) message return code (depended to Msg type).
     */
    static LRESULT CALLBACK WinFunc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );

    /* System exit function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    VOID DoExit( VOID );

    /* Enable/disable full screen window mode function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    VOID ToggleFullScreen( VOID )
    {
      IsChanged = !IsChanged;
    } /* End of 'ToggleFullScreen' function */

    /***
     * Cracked message handle functions
     ***/

    /* WM_CREATE window message handle function.
     * ARGUMENTS:
     *   - structure with creation data:
     *       CREATESTRUCT *CS;
     * RETURNS:
     *   (BOOL) TRUE to continue creation window, FALSE to terminate.
     */
    BOOL OnCreate( CREATESTRUCT *CS );

    /* WM_DESTROY window message handle function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    VOID OnDestroy( VOID );

    /* WM_SIZE window message handle function.
     * ARGUMENTS:
     *   - sizing flag (see SIZE_***, like SIZE_MAXIMIZED)
     *       UINT State;
     *   - new width and height of client area:
     *       INT W, H;
     * RETURNS: None.
     */
    VOID OnSize( UINT State, INT W, INT H );

    /* WM_ERASEBKGND window message handle function.
     * ARGUMENTS:
     *   - device context of client area:
     *       HDC hDC;
     * RETURNS:
     *   (BOOL) TRUE if background is erased, FALSE otherwise.
     */
    BOOL OnEraseBkgnd( HDC hDC );

    /* WM_PAINT window message handle function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    VOID OnPaint( VOID );

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
    VOID OnActivate( UINT Reason, HWND hWndActDeact, BOOL IsMinimized );

    /* WM_TIMER window message handle function.
     * ARGUMENTS:
     *   - specified the timer identifier.
     *       INT Id;
     * RETURNS: None.
     */
    VOID OnTimer( INT Id );

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
    VOID OnButtonDown( BOOL IsDoubleClick, INT X, INT Y, UINT Keys );

    /* WM_*BUTTONUP window message handle function.
     * ARGUMENTS:
     *   - mouse window position:
     *       INT X, Y;
     *   - mouse keys bits (see MK_*** bits constants):
     *       UINT Keys;
     * RETURNS: None.
     */
    VOID OnButtonUp( INT X, INT Y, UINT Keys );

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
    VOID OnMouseWheel( INT X, INT Y, INT Z, UINT Keys );

    /* Get window size function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (std::pair<INT, INT>) size of window.
     */
    std::pair<INT, INT> GetSize( VOID )
    {
      return std::pair<INT, INT>(Width, Height);
    } /* End of 'GetSize' function */

    /***
     * Virtual functions for window customization
     ***/

    /* Initialization function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID Init( VOID )
    {
    } /* End of 'Init' function */

    /* Deinitialization function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID Close( VOID )
    {
    } /* End of 'Close' function */

    /* Change window size handle function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID Resize( VOID )
    {
    } /* End of 'Resize' function */

    /* Erase background handle function.
     * ARGUMENTS:
     *   - device context of client area:
     *       HDC hDC;
     * RETURNS: None.
     */
    virtual VOID Erase( HDC hDC )
    {
    } /* End of 'Erase' function */

    /* Paint window content function.
     * ARGUMENTS:
     *   - device context of client area:
     *       HDC hDC;
     * RETURNS: None.
     */
    virtual VOID Paint( HDC hDC )
    {
    } /* End of 'Paint' function */

    /* Activate handle function.
     * ARGUMENTS:
     *   - activation window flag:
     *       BOOL IsActive;
     * RETURNS: None.
     */
    virtual VOID Activate( BOOL IsActive )
    {
    } /* End of 'Activate' function */

    /* Timer handle function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID Timer( VOID )
    {
    } /* End of 'Timer' function */

    /* Free CPU time handling function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID Idle( VOID )
    {
    } /* End of 'Idle' function */
  }; /* End of 'win' class */
} /* end of 'top' namespace */
