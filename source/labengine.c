#include "labengine.h"

#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>
#include <strsafe.h>

#ifdef _DEBUG 
#define LAB_ENABLE_REPORT
#endif

typedef struct lab_globals
{
  boolean_t init;
  DWORD threadId;
  HANDLE thread;
  HANDLE syncEvent;
  HWND hwnd;
  boolean_t quit;
  LONG width; // initialization in LabInit()
  LONG height;
  HBITMAP hbm;
  HDC hbmdc;
  HRGN hrgn;
} lab_globals;

static lab_globals s_globals = {
  LAB_FALSE,    // init
  ~0,           // threadId
  NULL,         // thread
  NULL,         // syncEvent
  NULL,         // hwnd
  LAB_FALSE,    // quit
};

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Error report
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void _labReportError()
{
  #ifdef LAB_ENABLE_REPORT

  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;
  DWORD dw = GetLastError(); 

  FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dw,
      MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
      (LPTSTR) &lpMsgBuf,
      0, NULL );

  // Display the error message and exit the process
  if (dw != 0)
  {
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("Failed with error %d: %s"), dw, lpMsgBuf); 
    OutputDebugString((LPTSTR) lpDisplayBuf);
    // MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 
    LocalFree(lpDisplayBuf); 
  }
  LocalFree(lpMsgBuf);

#endif LAB_ENABLE_REPORT
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Message handlers
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static LRESULT _onClose(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  if (s_globals.quit)
  {
    DestroyWindow(hwnd);
    DeleteObject(s_globals.hbmdc);
  }
  return 0;
}

static LRESULT _onDestroy(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  PostQuitMessage(0);
  return 0;
}

static LRESULT _onPaint(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;
  DWORD res;
  hdc = BeginPaint(hwnd, &ps);
  if (hdc)
  {
    res = BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, s_globals.hbmdc, 0, 0, SRCCOPY);
    if (!res)
      _labReportError();
  }
  EndPaint(hwnd, &ps);
  return 0;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Graphics
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LabDrawLine(int x1, int y1,  int x2, int y2)
{
  RECT r;
  r.left = x1;
  r.right = x2;
  r.top = y1;
  r.bottom = y2;
  // define region to redraw
  MoveToEx(s_globals.hbmdc, x1, y1, NULL);
  LineTo(s_globals.hbmdc, x2, y2);
  InvalidateRect(s_globals.hwnd, &r, FALSE);
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Window procedure
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK _labWindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  LRESULT ret = 0;

#define HANDLE_MESSAGE(msg, func) \
  case msg: \
  ret = func(hwnd, uMsg, wParam, lParam); \
  break;

  switch (uMsg)
  {
    HANDLE_MESSAGE(WM_CLOSE, _onClose);
    HANDLE_MESSAGE(WM_DESTROY, _onDestroy);
    HANDLE_MESSAGE(WM_PAINT, _onPaint);

  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }

#undef HANDLE_MESSAGE

  return 0;
}

static HWND _labCreateWindow(void)
{
  static LPCTSTR MY_CLASS_NAME = TEXT("LabWindowClass");
  static LPCTSTR MY_WINDOW_NAME = TEXT("Lab Graphics");
  WNDCLASSEX wcex;
  HWND hwnd = NULL;
  RECT rc = {0, 0, s_globals.width, s_globals.height};
  DWORD style = (WS_OVERLAPPEDWINDOW  & ~WS_THICKFRAME  & ~WS_MAXIMIZEBOX) | WS_VISIBLE; /*disable maximize and sising button*/
  HINSTANCE hInstance = GetModuleHandle(0);

  // Register class
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE; /*disable [X]*/
  wcex.lpfnWndProc = _labWindowProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = MY_CLASS_NAME;
  wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
  if (!RegisterClassEx(&wcex))
  {
    _labReportError();
    return NULL;
  }
    
  // create window
  AdjustWindowRect(&rc, style, FALSE);
  hwnd = CreateWindow(MY_CLASS_NAME, MY_WINDOW_NAME, style,
    CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
  if (hwnd == NULL)
    _labReportError();
  return hwnd;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Thread procedure
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static DWORD WINAPI _labThreadProc(_In_ LPVOID lpParameter)
{
  HDC hdc;
  RECT rect = {0, 0, s_globals.width, s_globals.height};

  // create window
  s_globals.hwnd = _labCreateWindow();
  if (s_globals.hwnd == NULL)
    _labReportError();

  // create second frame buffer
  hdc = GetDC(s_globals.hwnd);
  s_globals.hbmdc = CreateCompatibleDC(hdc);
  if (!s_globals.hbmdc)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    _labReportError();
  }
  s_globals.hbm = CreateCompatibleBitmap(hdc, s_globals.width, s_globals.height);
  if (!s_globals.hbm)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    _labReportError();
  }
  SelectObject(s_globals.hbmdc, s_globals.hbm);
  ReleaseDC(s_globals.hwnd, hdc);

  // initialize pen and background colors
  SelectObject(s_globals.hbmdc, GetStockObject(BLACK_PEN));
  SelectObject(s_globals.hbmdc, GetStockObject(HOLLOW_BRUSH));
  FillRect(s_globals.hbmdc, &rect, (HBRUSH) (HOLLOW_BRUSH));
  InvalidateRect(s_globals.hwnd, NULL, TRUE);

  // synchronize with the main thread
  SetEvent(s_globals.syncEvent);
  if (!s_globals.hwnd)
  {
    _labReportError();
    return 1;
  }

  // message loop
  {
    MSG msg;
    BOOL ret;
    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
      if (ret > 0)
      {
        TranslateMessage(&msg); 
        DispatchMessage(&msg);
      }
      else
      {
        _labReportError();
        break;
      }
    }
  }

  // exit thread
  return 0;
}



// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Initialization and termination routines
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static void _labThreadCleanup(void)
{
  DWORD res;
  if (s_globals.thread != NULL)
  {
    // destroy the thread
    res = CloseHandle(s_globals.thread);
    if (!res)
      _labReportError();
    s_globals.thread = NULL;
    s_globals.threadId = ~0;
  }

  // destroy the synchronization object
  if (s_globals.syncEvent)
  {
    res = CloseHandle(s_globals.syncEvent);
    if (!res)
      _labReportError();
    s_globals.syncEvent = NULL;
  }
}

boolean_t LabInit(void)
{
  DWORD res;
  LPTHREAD_START_ROUTINE lpStartAddress = NULL;

  // do not initialize twice
  if (s_globals.init)
    return LAB_FALSE;

  s_globals.width = 640;
  s_globals.height = 480;
  
  // create synchronization object
  s_globals.syncEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("LabSyncEvent"));
  if (!s_globals.syncEvent)
    goto on_error;

  // create thread for message processing and run it
  if (_labThreadProc == NULL)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    goto on_error;
  }
  s_globals.thread = CreateThread(NULL, 0, _labThreadProc , NULL, 0, &s_globals.threadId);
  if (!s_globals.thread)
    goto on_error;

  // wait until the window is created in another thread
  res = WaitForSingleObject(s_globals.syncEvent, INFINITE);
  if ((!s_globals.hwnd) || (res == WAIT_FAILED))
    goto on_error;

  // success
  s_globals.init = LAB_TRUE;
  return LAB_TRUE;

on_error:
  _labThreadCleanup();
  _labReportError();
  return LAB_FALSE;
}

void LabTerm(void)
{
  DWORD res;

  if (!s_globals.init)
    return;

  s_globals.quit = LAB_TRUE;

  // request the thread to terminate by closing the window
  _ASSERT(s_globals.hwnd != NULL);
  PostMessage(s_globals.hwnd, WM_CLOSE, 0, 0);

  // wait for the thread to finish
  _ASSERT(s_globals.thread);
  res = WaitForSingleObject(s_globals.thread, INFINITE); // todo: get rid of INFINITE [9/30/2013 paul.smirnov]
  if (res == WAIT_FAILED)
    _labReportError();
  // done
  _labThreadCleanup();
  s_globals.init = LAB_FALSE;
}

//get width and height of the window
int LabGetWidth(void)
{
  return s_globals.width;
}

int LabGetHeight(void)
{
  return s_globals.height;
}


// End of file
