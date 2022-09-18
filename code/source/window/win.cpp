#include <p_header.h>
#include "imgui/imgui_impl_win32.h"

/* Default constructor. */
gdr::win::win( VOID ) : IsInit(FALSE), hWnd(NULL), hInstance(GetModuleHandle(NULL)),
    Width(0), Height(0), IsActive(FALSE),
    IsFullScreen(FALSE), IsChanged(FALSE), MouseWheel(0)
{
  /* Fill and register window class */
  WNDCLASSEX wc;
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = sizeof(win *);
  wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hIconSm = (HICON)LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON,
  	GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
  wc.hIcon = (HICON)LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON,
  	GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
  wc.lpszMenuName = NULL;
  wc.hInstance = hInstance;
  wc.lpfnWndProc = WinFunc;
  wc.lpszClassName = MainWndClassName;
  if (!RegisterClassEx(&wc))
  {
    MessageBox(NULL, L"Error register window class", L"ERROR",
      MB_OK | MB_ICONSTOP);
    exit(0);
  }
  
  /* Window creation */
  hWnd = CreateWindow(MainWndClassName,
    L"GDR",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    NULL, NULL, hInstance, (VOID *)this);
  if (hWnd == NULL)
  {
    MessageBox(NULL, L"Error window creation", L"ERROR",
      MB_OK | MB_ICONSTOP);
    exit(0);
  }

  /* Show and update window */
  ShowWindow(hWnd, SW_SHOWNORMAL);
  UpdateWindow(hWnd);
  
  /* Send initialization timer message */
  PostMessage(hWnd, WM_TIMER, (WPARAM)InitializationTimer, 0);
}

/* Write text in header function.
  * ARGUMENTS: None.
  *    - string to write;
  *       std::string &Str;
  * RETURNS: None.
  */
VOID gdr::win::WriteHeader( const std::string &Str )
{
  std::wstring Wide = std::wstring(Str.begin(), Str.end());
  SetWindowText(hWnd, Wide.c_str());
}

/* Class destructor */
gdr::win::~win( VOID )
{
}

/* Windowed application running function.
 * ARGUMENTS: None.
 * RETURNS:
 *   (INT) Error level for operation system (0 for success).
 */
WPARAM gdr::win::Run( VOID )
{
  MSG msg;
  
  /* Message loop */
  while (TRUE)
  {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
        break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
      if (IsInit)
        Idle();
  }
  return msg.wParam;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
LRESULT CALLBACK gdr::win::WinFunc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
  win *Win;
  
  if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
    return true;

  switch (Msg)
  {
  case WM_CREATE:/* Attach 'this' pointer to window class to window */
    SetWindowLongPtr(hWnd, 0, (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
    /* Store window handle */
    ((win *)((CREATESTRUCT *)lParam)->lpCreateParams)->hWnd = hWnd;
  default:
    Win = (win *)GetWindowLongPtr(hWnd, 0);
    if (Win != NULL)
    {
      switch (Msg)
      {
      case WM_CREATE:
        return Win->OnCreate((CREATESTRUCT *)lParam) ? 0 : -1;
      case WM_DESTROY:
        Win->OnDestroy();
        return 0;
      case WM_SIZE:
        Win->OnSize((UINT)wParam, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam));
        return 0;
      case WM_ACTIVATE:
        Win->OnActivate((UINT)LOWORD(wParam), (HWND)(lParam), (BOOL)HIWORD(wParam));
        return 0;
      case WM_ERASEBKGND:
        return (LRESULT)Win->OnEraseBkgnd((HDC)wParam);
      case WM_PAINT:
        Win->OnPaint();
        return 0;
      case WM_TIMER:
        Win->OnTimer((UINT)wParam);
        return 0;
      case WM_LBUTTONDOWN:
      case WM_RBUTTONDOWN:
      case WM_MBUTTONDOWN:
        Win->OnButtonDown(FALSE, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(SHORT)LOWORD(wParam));
        return 0;
      case WM_CHAR:
        if (Win->CharAdapter != NULL) 
          Win->CharAdapter((CHAR)wParam);
        return 0;
      case WM_KEYDOWN:
        if (Win->KeydownAdapter != NULL) 
          Win->KeydownAdapter((CHAR)wParam);
        return 0;
      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP:
        Win->OnButtonUp((INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (UINT)(SHORT)LOWORD(wParam));
        return 0;
      case WM_MOUSEWHEEL:
        Win->OnMouseWheel((INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (INT)(SHORT)HIWORD(wParam), (UINT)(SHORT)LOWORD(wParam));
        return 0;
      case WM_SYSCHAR:
        return 0;
      }
      return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
  }
  return DefWindowProc(hWnd, Msg, wParam, lParam);
}

/* System exit function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::win::DoExit( VOID )
{
  PostMessage(hWnd, WM_CLOSE, 0, 0);
}

/* Enable/disable full screen window mode function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID gdr::win::SetFullScreen( VOID )
{
  if (!IsFullScreen)
  {
    RECT rc;
    HMONITOR hmon;
    MONITORINFOEX moninfo;

    /* save window style */
    SaveStyle = GetWindowLong(hWnd, GWL_STYLE);

    /* save old window size */
    GetWindowRect(hWnd, &SaveRect);

    /* determine monitor with window */
    hmon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    
    /* obtain monitor information */
    moninfo.cbSize = sizeof(moninfo);
    GetMonitorInfo(hmon, (MONITORINFO *)&moninfo);

    /* go to full screen mode */
    rc = moninfo.rcMonitor;

    SetWindowLong(hWnd, GWL_STYLE,
                  SaveStyle & ~WS_OVERLAPPEDWINDOW);

    SetWindowPos(hWnd, HWND_TOPMOST,
      rc.left, rc.top,
      rc.right - rc.left, rc.bottom - rc.top,
      SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    IsFullScreen = TRUE;
  }
  else
  {
    /* restore window style */
    SetWindowLong(hWnd, GWL_STYLE,
                  SaveStyle);

    /* restore window size */
    SetWindowPos(hWnd, HWND_NOTOPMOST,
      SaveRect.left, SaveRect.top,
      SaveRect.right - SaveRect.left, SaveRect.bottom - SaveRect.top,
      SWP_NOOWNERZORDER);
    IsFullScreen = FALSE;
  }
}
