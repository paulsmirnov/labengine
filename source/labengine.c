/** @file labengine.c
 * LabEngine library implementation.
 */

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

/**
 * structure for keyboard buffer implementation
 */
typedef struct lab_queue
{
  /// index of first element in queue
  int start;
  /// index next to the last element in queue
  int end;
  /// circular queue
  int key[BUFFER_SIZE];
} lab_queue;

/**
 * structure which contains information about threads, window and other objects
 */
typedef struct lab_globals
{
  /// if value is LAB_TRUE, graphics mode has already been initialized
  labbool_t init;
  /// receives the thread identifier when thread creates in <code>CreateThread()</code> function
  DWORD threadId;
  /// handle to a new thread - return value of <code>CreateThread()</code> function
  HANDLE thread;
  /// event used to synchronize with the main thread
  HANDLE syncEvent;
  /// handle to a window
  HWND hwnd;
  /// if value is LAB_TRUE, window will be destroyed and graphics mode will be closed
  labbool_t quit;
  /// width of window
  LONG width;
  /// height of window
  LONG height;
  /// a handle to a bitmap used to draw
  HBITMAP hbm;
  /// a handle to device context of hbm
  HDC hbmdc;
  /// a handle to a region to be updated after drawing
  HRGN hrgn;
  /// critical section object used to provide sinchronization in graphics
  CRITICAL_SECTION cs;
  /// semaphore object used to provide sinchronization in input system
  HANDLE ghSemaphore;
  /// array of colors (array of rgb)
  COLORREF colors[LABCOLOR_COUNT];
  /// current pen color
  labcolor_t penColor;
  /// current rgb pen color
  COLORREF penColorRGB;
  /// update area
  RECT updateRect;
} lab_globals;

static lab_globals s_globals = {
  LAB_FALSE,    // init
  ~0,           // threadId
  NULL,         // thread
  NULL,         // syncEvent
  NULL,         // hwnd
  LAB_FALSE,    // quit
};

static lab_queue key_queue = {
  0,            // start
  0,            // end
  0,            // key[]
};

static COLORREF s_default_colors[] = {
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
//   Queue functionality
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

labbool_t _labFullQue(void)
{
  return ((key_queue.start == key_queue.end + 1) || (key_queue.end - key_queue.start == BUFFER_SIZE - 1)) ? LAB_TRUE : LAB_FALSE;
}

labbool_t _labEmptyQue(void)
{
  return (key_queue.start == key_queue.end) ? LAB_TRUE : LAB_FALSE;
}

labbool_t _labPush(int c)
{
  if (!_labFullQue())
  {
    key_queue.key[key_queue.end] = c;
    // make it circular
    if (key_queue.end + 1 == BUFFER_SIZE)
      key_queue.end = 0;
    else
      key_queue.end++;
  }
  else
    MessageBeep(0xFFFFFFFF); // if queue is full, don't add element, just beep
  if (!ReleaseSemaphore(s_globals.ghSemaphore, 1, NULL)) // increase semaphore oblect value by one
    _labReportError();
  return _labFullQue();
}

int _labPop(void)
{
  int key;
  if (!_labEmptyQue())
  {
    key = key_queue.key[key_queue.start];
    // pop elemnt
    key_queue.key[key_queue.start] = 0;
    // make it circular
    if (key_queue.start + 1 == BUFFER_SIZE)
      key_queue.start = 0;
    else
      key_queue.start++;
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
	EnterCriticalSection(&s_globals.cs);
//	if (TryEnterCriticalSection(&s_globals.cs))
	{
		hdc = BeginPaint(hwnd, &ps);
		if (hdc)
		{
			//if (IsRectEmpty(&s_globals.updateRect))
				CopyRect(&s_globals.updateRect, &ps.rcPaint);
			res = BitBlt(hdc,
				s_globals.updateRect.left, s_globals.updateRect.top,
				s_globals.updateRect.right, s_globals.updateRect.bottom,
				s_globals.hbmdc,
				s_globals.updateRect.left, s_globals.updateRect.top,
				SRCCOPY);
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

    for (i = 0; i < (mask & lParam); i++) // in lParam bits from 0 to 15 - the repeat count for the current message
      if (_labPush(code)) // break if queue is full
        break;
  return 0;
}

// other keys processing
static LRESULT _onKeydown(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  int i;
  int virtual_code;
  int mask = 0x0000FFFF; // 00..011..1
  switch (wParam) {

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
  {
    if (_labPush(virtual_code))
      break;
  }
  return 0;
}


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Input system
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
 * Waits until key pressed and push key pressed code to the keyboard buffer
 *
 * @return integer value - code of the key that was pressed
 */
int LabInputKey(void)
{
  LABASSERT_INIT();
  // waits until key pressed in another thread and decreases semaphore object
  WaitForSingleObject(s_globals.ghSemaphore, INFINITE); 
//  InvalidateRect(s_globals.hwnd, NULL, FALSE);
  return _labPop();
}


/** 
 * @brief Checks if there are any keys in the keyboard buffer
 *
 * Buffer is not empty means that it contains not processed by user who calls this function keystrokes
 * @return LAB_TRUE if buffer is not empty and keystroke is ready to be processed, else return value is LAB_FALSE
 */
labbool_t LabInputKeyReady(void)
{
  LABASSERT_INIT();
  return (_labEmptyQue() == LAB_FALSE) ? LAB_TRUE : LAB_FALSE;
}


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Graphics
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
 * Initializes colors components (r, g, and b)
 */
void _labInitColors(void)
{
	_STATIC_ASSERT(sizeof(s_default_colors) == sizeof(s_globals.colors));
	_STATIC_ASSERT(sizeof(s_default_colors)/sizeof(s_default_colors[0]) == LABCOLOR_COUNT);
	memcpy(s_globals.colors, s_default_colors, sizeof(s_globals.colors));
}

/** 
 * Sets new line color
 *
 * @param color new line color
 */
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

/** 
 * Get current line color
 *
 * @retrun integer value - current color
 */
int LabGetColor(void)
{
  LABASSERT_INIT();
  return s_globals.penColor;
}

/** 
 * Draw line which joins point (x1, y1) and (x2, y2).
 *
 * @param x1 x-coordinate of first point
 * @param y1 y-coordinate of first point
 * @param x2 x-coordinate of second point
 * @param y2 y-coordinate of second point
 */
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

/** 
 * Draw point with coordinates (x, y)
 *
 * @param x x-coordinate of point
 * @param y y-coordinate of point
 */
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

/** 
 * Draw circle which center in (x, y) and radius radius.
 *
 * @param x x-coordinate of center
 * @param y y-coordinate of center
 * @param radius radius
 */
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

/** 
 * Draw ellipse which center in (x, y) and semiaxises a and b.
 *
 * @param x x-coordinate of center
 * @param y y-coordinate of center
 * @param a semi-major axis
 * @param b semi-minor axis
 */
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

/** 
 * Draw rectangle on upper-left corner with coordinate (x1, y1) and bottom-right (x2, y2).
 *
 * @param x1 x-coordinate of upper-left corner
 * @param y1 y-coordinate of upper-left corner
 * @param x2 x-coordinate of bottom-right corner
 * @param y2 y-coordinate of bottom-right corner
 */
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

/**
 * @brief Initializes graphics.
 * 
 * Initializes structure lab_globals if it is not initialized yet.
 * Creates thread for message processing and runs it.
 *
 * @return LAB_TRUE if initialization was successful, otherwise return value is LAB_FALSE
 */
labbool_t LabInit(void)
{
  DWORD res;
  LPTHREAD_START_ROUTINE lpStartAddress = NULL;

  // do not initialize twice
  LABASSERT(!s_globals.init);
  if (s_globals.init)
    return LAB_FALSE;

  s_globals.width = 640;
  s_globals.height = 480;

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



/**
 * @brief Terminates work with graphics
 * 
 * Requests the tread to terminate by closing the window and destroys the thread.
 */
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


/**
 * @brief Gets window width.
 * 
 * Takes current window width from structure lab_globals and returns token value.
 * @return an integer value - window width
 * @see LabGetHeight
 */
int LabGetWidth(void)
{
  LABASSERT_INIT();
  return s_globals.width;
}


/**
 * @brief Gets window height.
 * 
 * Takes current window height from structure lab_globals and returns token value.
 * @return an integer value - window height
 * @see LabGetWidth
 */
int LabGetHeight(void)
{
  LABASSERT_INIT();
  return s_globals.height;
}

/**
 * @brief Clears window with black color.
 * 
 * @see LabClearWith
 */
void LabClear()
{
  LABASSERT_INIT();
  LabClearWith(LABCOLOR_BLACK);
}

/**
 * @brief Clears window with specified color.
 * 
 * @param color clear color
 * @see LabClear
 */
void LabClearWith(labcolor_t color)
{
  HBRUSH colorBrush = CreateSolidBrush(s_globals.colors[color]); // todo: why not SetDCBrushColor()? [4/3/2015 paul.smirnov]
  RECT screenRect;

  LABASSERT_INIT();
  SetRect(&screenRect, 0, 0, LabGetWidth(), LabGetHeight());

  EnterCriticalSection(&s_globals.cs);
  {
    //UnionRect(&s_globals.updateRect, &s_globals.updateRect, &screenRect);
    FillRect(s_globals.hbmdc, &screenRect, colorBrush);
    LeaveCriticalSection(&s_globals.cs);
  }

  DeleteObject(colorBrush);
}


// End of file
