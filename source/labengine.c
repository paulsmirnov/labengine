#include "labengine.h"

#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>
#include <strsafe.h>

#define BUFFER_SIZE 32 /// size of queue for keyboard buffer (31 + 1)
#define MAX_SEM_COUNT BUFFER_SIZE /// max semaphore object count

#define LABASSERT(e)      _ASSERTE(e)
#define LABASSERT_INIT()  LABASSERT("LabInit() should be called first!" && s_globals.init);

#ifdef _DEBUG 
#define LAB_ENABLE_REPORT
#endif

typedef struct labkeyqueue_t
{
  int start; // index of first element in queue
  int end;   // index next to the last element in queue
  int key[BUFFER_SIZE]; // circular queue
} labkeyqueue_t;

typedef struct labglobals_t
{
  labbool_t init;       // if value is LAB_TRUE, graphics mode has already been initialized

  DWORD threadId;       // receives the thread identifier when thread creates in <code>CreateThread()</code> function
  HANDLE thread;        // handle to a new thread - return value of <code>CreateThread()</code> function
  HANDLE syncEvent;     // event used to synchronize with the main thread

  HWND hwnd;            // handle to a window

  labbool_t quit;       // if value is LAB_TRUE, window will be destroyed and graphics mode will be closed

  LONG width;           // width of window
  LONG height;          // height of window
  DWORD scale;          // scale factor for buffer output

  HBITMAP hbm;          // a handle to a bitmap used to draw
  HDC hbmdc;            // a handle to device context of hbm
  HRGN hrgn;            // a handle to a region to be updated after drawing
  
  CRITICAL_SECTION cs;  // critical section object used to provide sinchronization in graphics
  HANDLE ghSemaphore;   // semaphore object used to provide sinchronization in input system

  COLORREF colors[LABCOLOR_COUNT];  // array of colors (array of rgb)
  labcolor_t penColor;  // current pen color
  COLORREF penColorRGB; // current rgb pen color

  RECT updateRect;      // update area
} labglobals_t;

static labglobals_t s_globals = {
  LAB_FALSE,    // init
  ~0,           // threadId
  NULL,         // thread
  NULL,         // syncEvent
  NULL,         // hwnd
  LAB_FALSE,    // quit
};

static labkeyqueue_t s_keyQueue;

static COLORREF s_defaultColors[] = {
  RGB(   0,   0,   0), // LABCOLOR_BLACK,
  RGB(   0,   0, 128), // LABCOLOR_DARK_BLUE,
  RGB(   0, 128,   0), // LABCOLOR_DARK_GREEN,
  RGB(   0, 128, 255), // LABCOLOR_DARK_CYAN,
  RGB( 128,   0,   0), // LABCOLOR_DARK_RED,
  RGB( 128,   0, 128), // LABCOLOR_DARK_MAGENTA,
  RGB( 128,  64,   0), // LABCOLOR_BROWN,
  RGB( 192, 192, 192), // LABCOLOR_LIGHT_GREY,
  RGB( 128, 128, 128), // LABCOLOR_DARK_GREY,
  RGB(   0,   0, 255), // LABCOLOR_BLUE,
  RGB(   0, 255,   0), // LABCOLOR_GREEN,
  RGB(   0, 255, 255), // LABCOLOR_CYAN,
  RGB( 255,   0,   0), // LABCOLOR_RED,
  RGB( 255,   0, 255), // LABCOLOR_MAGENTA,
  RGB( 255, 255,   0), // LABCOLOR_YELLOW,
  RGB( 255, 255, 255), // LABCOLOR_WHITE,
};

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Forward declarations
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static __inline int _labGetWindowWidth(void);
static __inline int _labGetWindowHeight(void);

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Error report
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void _labReportError()
{
#ifdef LAB_ENABLE_REPORT

  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;
  DWORD dw = GetLastError(); 

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, dw, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

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
//   Queue functionality
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

labbool_t _labInputQueueFull(void)
{
  return ((s_keyQueue.start == s_keyQueue.end + 1) || (s_keyQueue.end - s_keyQueue.start == BUFFER_SIZE - 1)) ? LAB_TRUE : LAB_FALSE;
}

labbool_t _labInputQueueEmpty(void)
{
  return (s_keyQueue.start == s_keyQueue.end) ? LAB_TRUE : LAB_FALSE;
}

labbool_t _labInputKeyPush(int c)
{
  if (!_labInputQueueFull())
  {
    s_keyQueue.key[s_keyQueue.end] = c;
    // make it circular
    if (s_keyQueue.end + 1 == BUFFER_SIZE)
      s_keyQueue.end = 0;
    else
      s_keyQueue.end++;
  }
  else
    MessageBeep(0xFFFFFFFF); // if queue is full, don't add element, just beep
  if (!ReleaseSemaphore(s_globals.ghSemaphore, 1, NULL)) // increase semaphore oblect value by one
    _labReportError();
  return !_labInputQueueFull();
}

int _labInputKeyPop(void)
{
  int key;
  if (!_labInputQueueEmpty())
  {
    key = s_keyQueue.key[s_keyQueue.start];
    // pop elemnt
    s_keyQueue.key[s_keyQueue.start] = 0;
    // make it circular
    if (s_keyQueue.start + 1 == BUFFER_SIZE)
      s_keyQueue.start = 0;
    else
      s_keyQueue.start++;
    return key;
  }
  return 0; // if element was not deleted (queue was empty)
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
    DeleteCriticalSection(&s_globals.cs);
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
  RECT srcRect, dstRect;

  EnterCriticalSection(&s_globals.cs);
  //	if (TryEnterCriticalSection(&s_globals.cs))
  {
    hdc = BeginPaint(hwnd, &ps);
    if (hdc)
    {
      //if (IsRectEmpty(&s_globals.updateRect))
      CopyRect(&dstRect, &ps.rcPaint);
      if (s_globals.scale == 1)
      {
        res = BitBlt(hdc, dstRect.left, dstRect.top, dstRect.right, dstRect.bottom,
          s_globals.hbmdc, dstRect.left, dstRect.top, SRCCOPY);
      }
      else
      {
        // find source rectangle
        srcRect.left = dstRect.left / s_globals.scale;
        srcRect.right = (dstRect.right + s_globals.scale - 1) / s_globals.scale;
        srcRect.top = dstRect.top / s_globals.scale;
        srcRect.bottom = (dstRect.bottom + s_globals.scale - 1) / s_globals.scale;

        // corresponding screen rectangle (rounded up)
        dstRect.left = srcRect.left * s_globals.scale;
        dstRect.right = srcRect.right * s_globals.scale;
        dstRect.top = srcRect.top * s_globals.scale;
        dstRect.bottom = srcRect.bottom * s_globals.scale;

        res = StretchBlt(hdc, dstRect.left, dstRect.top, dstRect.right, dstRect.bottom,
          s_globals.hbmdc, srcRect.left, srcRect.top, srcRect.right, srcRect.bottom, SRCCOPY);
      }
      if (!res)
        _labReportError();
      SetRectEmpty(&s_globals.updateRect);
    }
    LeaveCriticalSection(&s_globals.cs);
  }
  EndPaint(hwnd, &ps);
  return 0;
}

// button faces and 4 special keys processing
static LRESULT _onChar(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  int i;
  int code;
  int mask = 0x0000FFFF; // 00..011..1

  // special codes for these keys
  switch (wParam)  
  {
  case VK_RETURN: // ENTER key
    code = LABKEY_ENTER;
    break;

  case VK_ESCAPE:
    code = LABKEY_ESC;
    break;

  case VK_BACK:
    code = LABKEY_BACK;
    break;

  case VK_TAB:
    code = LABKEY_TAB;
    break;

  default:
    {
      // button faces processing
      code = wParam;
      break;
    }
  }

  for (i = 0; i < (mask & lParam); i++) // repeat count for the current message
    if (!_labInputKeyPush(code))
      break;
  return 0;
}

// other keys processing
static LRESULT _onKeydown(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  int i;
  int virtual_code;
  int mask = 0x0000FFFF; // 00..011..1

  switch (wParam)
  {
  case VK_LEFT:
    virtual_code = LABKEY_LEFT;
    break;

  case VK_UP:
    virtual_code = LABKEY_UP;
    break;

  case VK_RIGHT:
    virtual_code = LABKEY_RIGHT;
    break;

  case VK_DOWN:
    virtual_code = LABKEY_DOWN;
    break;
  case VK_PRIOR: // PAGE UP key
    virtual_code = LABKEY_PAGE_UP;
    break;

  case VK_NEXT: // PAGE DOWN key
    virtual_code = LABKEY_PAGE_DOWN;
    break;

  default:
    return 0;
  }

  for (i = 0; i < (mask & lParam); i++)
    if (!_labInputKeyPush(virtual_code))
      break;
  return 0;
}


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Input system
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LabInputKey(void)
{
  LABASSERT_INIT();
  // waits until key pressed in another thread and decreases semaphore object
  WaitForSingleObject(s_globals.ghSemaphore, INFINITE); 
//  InvalidateRect(s_globals.hwnd, NULL, FALSE);
  return _labInputKeyPop();
}

labbool_t LabInputKeyReady(void)
{
  LABASSERT_INIT();
  return (_labInputQueueEmpty() == LAB_FALSE) ? LAB_TRUE : LAB_FALSE;
}


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Graphics
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void _labInitColors(void)
{
  _STATIC_ASSERT(sizeof(s_defaultColors) == sizeof(s_globals.colors));
  _STATIC_ASSERT(sizeof(s_defaultColors)/sizeof(s_defaultColors[0]) == LABCOLOR_COUNT);
  memcpy(s_globals.colors, s_defaultColors, sizeof(s_globals.colors));
}

void LabSetColor(labcolor_t color)
{
  LABASSERT_INIT();
  s_globals.penColor = color;
  s_globals.penColorRGB = s_globals.colors[color];
  SelectObject(s_globals.hbmdc, GetStockObject(DC_PEN));
  SetDCPenColor(s_globals.hbmdc, s_globals.penColorRGB);
}

void LabSetColorRGB(int r, int g, int b)
{
  LABASSERT_INIT();
  s_globals.penColor = LABCOLOR_NA;
  s_globals.penColorRGB = RGB(r & 0xFF, g & 0xFF, b & 0xFF);
  SelectObject(s_globals.hbmdc, GetStockObject(DC_PEN));
  SetDCPenColor(s_globals.hbmdc, s_globals.penColorRGB);
}

int LabGetColor(void)
{
  LABASSERT_INIT();
  return s_globals.penColor;
}

void LabDrawLine(int x1, int y1,  int x2, int y2)
{
  RECT r;

  LABASSERT_INIT();

  // define region to redraw
  r.left   = x1 <= x2 ? x1 : x2 + 1;
  r.right  = x1 <  x2 ? x2 : x1 + 1;
  r.top    = y1 <= y2 ? y1 : y2 + 1;
  r.bottom = y1 <  y2 ? y2 : y1 + 1;
//  if (TryEnterCriticalSection(&s_globals.cs))
  EnterCriticalSection(&s_globals.cs);
  {
    MoveToEx(s_globals.hbmdc, x1, y1, NULL);
    LineTo(s_globals.hbmdc, x2, y2);
    UnionRect(&s_globals.updateRect, &s_globals.updateRect, &r);
    //InvalidateRect(s_globals.hwnd, NULL, FALSE);
    LeaveCriticalSection(&s_globals.cs);
  }
}

void LabDrawPoint(int x, int y)
{
  RECT r;

  LABASSERT_INIT();

  // define region to redraw
  r.left   = x;
  r.right  = x + 1;
  r.top    = y;
  r.bottom = y + 1;
//  if (TryEnterCriticalSection(&s_globals.cs))
  EnterCriticalSection(&s_globals.cs);
  {
    SetPixel(s_globals.hbmdc, x, y, s_globals.penColorRGB); // draw point in current color
    UnionRect(&s_globals.updateRect, &s_globals.updateRect, &r);
    //InvalidateRect(s_globals.hwnd, NULL, FALSE);
    LeaveCriticalSection(&s_globals.cs);
  }
}

void LabDrawCircle(int x, int y,  int radius)
{
  RECT r;

  LABASSERT_INIT();

  // define region to redraw
  r.left   = x - radius;
  r.right  = x + radius + 1;
  r.top    = y - radius;
  r.bottom = y + radius + 1;
//  if (TryEnterCriticalSection(&s_globals.cs))
  EnterCriticalSection(&s_globals.cs);
  {
    SelectObject(s_globals.hbmdc, GetStockObject(NULL_BRUSH)); // not filled circle
    Ellipse(s_globals.hbmdc, x - radius, y - radius, x + radius, y + radius); 
    UnionRect(&s_globals.updateRect, &s_globals.updateRect, &r);
    //InvalidateRect(s_globals.hwnd, NULL, FALSE);
    LeaveCriticalSection(&s_globals.cs);
  }
}

void LabDrawEllipse(int x, int y,  int a, int b)
{
  RECT r;

  LABASSERT_INIT();

  // define region to redraw
  r.left   = x - a;
  r.right  = x + a + 1;
  r.top    = y - b;
  r.bottom = y + b + 1;
//  if (TryEnterCriticalSection(&s_globals.cs))
  EnterCriticalSection(&s_globals.cs);
  {
    SelectObject(s_globals.hbmdc, GetStockObject(NULL_BRUSH)); // not filled ellipse
    Ellipse(s_globals.hbmdc, x - a, y - b, x + a, y + b); 
    UnionRect(&s_globals.updateRect, &s_globals.updateRect, &r);
    //InvalidateRect(s_globals.hwnd, NULL, FALSE); // Если тут не NULL, а положенный &r, то обновляется медленно при удерживании клавиши.
    LeaveCriticalSection(&s_globals.cs);
  }
}

void LabDrawRectangle(int x1, int y1,  int x2, int y2)
{
  RECT r;

  LABASSERT_INIT();

  // define region to redraw
  r.left   = x1 < x2 ? x1 : x2;
  r.right  = x1 < x2 ? x2 : x1;
  r.top    = y1 < y2 ? y1 : y2;
  r.bottom = y1 < y2 ? y2 : y1;
//  if (TryEnterCriticalSection(&s_globals.cs))
  EnterCriticalSection(&s_globals.cs);
  {
    SelectObject(s_globals.hbmdc, GetStockObject(NULL_BRUSH)); // not filled rectangle
    Rectangle(s_globals.hbmdc, r.left, r.top, r.right, r.bottom);
    UnionRect(&s_globals.updateRect, &s_globals.updateRect, &r);
    //InvalidateRect(s_globals.hwnd, NULL, FALSE);
    LeaveCriticalSection(&s_globals.cs);
  }
}

void LabDrawFlush(void)
{
  LABASSERT_INIT();
  InvalidateRect(s_globals.hwnd, NULL, FALSE);
  UpdateWindow(s_globals.hwnd);
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
    HANDLE_MESSAGE(WM_KEYDOWN, _onKeydown);
    HANDLE_MESSAGE(WM_CHAR, _onChar);

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
  RECT rc = {0, 0, _labGetWindowWidth(), _labGetWindowHeight()};
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
  RECT rect = {0, 0, _labGetWindowWidth(), _labGetWindowHeight()};

  // create window
  s_globals.hwnd = _labCreateWindow();
  if (s_globals.hwnd == NULL)
    _labReportError();

  // create semaphore object
  s_globals.ghSemaphore = CreateSemaphore( 
        NULL,           // default security attributes
        0,              // initial count
        MAX_SEM_COUNT,  // maximum count
        NULL);          // unnamed semaphore

  if (s_globals.ghSemaphore == NULL)
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

  InitializeCriticalSection(&s_globals.cs);

  // initialize pen and background colors
  SelectObject(s_globals.hbmdc, GetStockObject(WHITE_PEN));
  SelectObject(s_globals.hbmdc, GetStockObject(BLACK_BRUSH));
  //FillRect(s_globals.hbmdc, &rect, (HBRUSH) (BLACK_BRUSH));

  // require to update the entire window first
  GetClientRect(s_globals.hwnd, &s_globals.updateRect);
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

labbool_t LabInit(void)
{
  labparams_t params;

  params.width = 640;
  params.height = 480;
  params.scale = 1;

  return LabInitWith(&params);
}

labbool_t LabInitWith(labparams_t const* params)
{
  DWORD res;
  LPTHREAD_START_ROUTINE lpStartAddress = NULL;

  // do not initialize twice
  LABASSERT(!s_globals.init);
  if (s_globals.init)
    return LAB_FALSE;

  LABASSERT((params->width > 0) && (params->height > 0));
  LABASSERT(params->scale > 0);
  s_globals.width = params->width;
  s_globals.height = params->height;
  s_globals.scale = params->scale;

  SetRectEmpty(&s_globals.updateRect);

  // initialize colors
  _labInitColors();

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

  // successfully initialized
  s_globals.init = LAB_TRUE;

  // set defaults now
  LabSetColor(LABCOLOR_WHITE);

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

void LabDelay(int time)
{
  LABASSERT_INIT();
  Sleep(time);
}

int LabGetWidth(void)
{
  LABASSERT_INIT();
  return s_globals.width;
}

static __inline int _labGetWindowWidth(void)
{
  return s_globals.width * s_globals.scale;
}

int LabGetHeight(void)
{
  LABASSERT_INIT();
  return s_globals.height;
}

static __inline int _labGetWindowHeight(void)
{
  return s_globals.height * s_globals.scale;
}

void LabClear()
{
  LABASSERT_INIT();
  LabClearWith(LABCOLOR_BLACK);
}

void LabClearWith(labcolor_t color)
{
  HBRUSH colorBrush = CreateSolidBrush(s_globals.colors[color]); // todo: why not SetDCBrushColor()? [4/3/2015 paul.smirnov]
  RECT screenRect = {0, 0, _labGetWindowWidth(), _labGetWindowHeight()};

  LABASSERT_INIT();

  EnterCriticalSection(&s_globals.cs);
  {
    //UnionRect(&s_globals.updateRect, &s_globals.updateRect, &screenRect);
    FillRect(s_globals.hbmdc, &screenRect, colorBrush);
    LeaveCriticalSection(&s_globals.cs);
  }

  DeleteObject(colorBrush);
}


// End of file
