#include "labengine.h"

#include <windows.h>
#include <crtdbg.h>

typedef struct lab_globals
{
  boolean_t init;
  DWORD threadId;
  HANDLE thread;
  HANDLE syncEvent;
  HWND hwnd;
  boolean_t quit;
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
//   Message handlers
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static LRESULT _onClose(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  if (s_globals.quit)
    DestroyWindow(hwnd);
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

  hdc = BeginPaint(hwnd, &ps);
  if (hdc)
  {
    // todo: remove this test line [9/30/2013 paul.smirnov]
    MoveToEx(hdc, 0, 0, NULL);
    LineTo(hdc, 640, 480);

    EndPaint(hwnd, &ps);
  }

  return 0;
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
  RECT rc = {0, 0, 640, 480};
  DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  HINSTANCE hInstance = GetModuleHandle(0);

  // Register class
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
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
    return NULL;
    
  // Create window
  AdjustWindowRect(&rc, style, FALSE);
  hwnd = CreateWindow(MY_CLASS_NAME, MY_WINDOW_NAME, style,
    CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

  return hwnd;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Thread procedure
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static DWORD WINAPI _labThreadProc(_In_ LPVOID lpParameter)
{
  // create window
  s_globals.hwnd = _labCreateWindow();
  
  // synchronize with the main thread
  SetEvent(s_globals.syncEvent);
  if (!s_globals.hwnd)
    return 1;

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
        // todo: process error [9/30/2013 paul.smirnov]
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
  if (s_globals.thread != NULL)
  {
    // destroy the thread
    CloseHandle(s_globals.thread);
    s_globals.thread = NULL;
    s_globals.threadId = ~0;
  }

  // destroy the synchronization object
  if (s_globals.syncEvent)
  {
    CloseHandle(s_globals.syncEvent);
    s_globals.syncEvent = NULL;
  }
}

boolean_t LabInit(void)
{
  // do not initialize twice
  if (s_globals.init)
    return LAB_FALSE;

  // create synchronization object
  s_globals.syncEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("LabSyncEvent"));
  if (!s_globals.syncEvent)
    goto on_error;

  // create thread for message processing and run it
  s_globals.thread = CreateThread(NULL, 0, _labThreadProc, NULL, 0, &s_globals.threadId);
  if (!s_globals.thread)
    goto on_error;

  // wait until the window is created in another thread
  WaitForSingleObject(s_globals.syncEvent, INFINITE);
  if (!s_globals.hwnd)
    goto on_error;

  // success
  s_globals.init = LAB_TRUE;
  return LAB_TRUE;

on_error:
  _labThreadCleanup();
  return LAB_FALSE;
}

void LabTerm(void)
{
  if (!s_globals.init)
    return;

  s_globals.quit = LAB_TRUE;

  // request the thread to terminate by closing the window
  _ASSERT(s_globals.hwnd != NULL);
  PostMessage(s_globals.hwnd, WM_CLOSE, 0, 0);

  // wait for the thread to finish
  _ASSERT(s_globals.thread);
  WaitForSingleObject(s_globals.thread, INFINITE); // todo: get rid of INFINITE [9/30/2013 paul.smirnov]

  // done
  _labThreadCleanup();
  s_globals.init = LAB_FALSE;
}

// End of file
