
#include "areaselecter.h"
#include <dwmapi.h>
#include <cassert>
#include <cstdio>
#include <algorithm>

using namespace common;

#define StrFormat(Buffer, ...) _snwprintf(Buffer, _countof(Buffer), __VA_ARGS__)

bool AreaSelector::init()
{
  const WCHAR className[100] = L"CreateDesktopRectWindow";

  // Initialize the window class.
  WNDCLASSEX windowClass = {0};
  windowClass.cbSize = sizeof(WNDCLASSEX);
  windowClass.style = CS_HREDRAW | CS_VREDRAW;
  windowClass.cbClsExtra = 0;
  windowClass.cbWndExtra = 0;
  windowClass.hIcon = NULL;
  windowClass.hIconSm = NULL;
  windowClass.lpfnWndProc = AreaSelector::StaticWindowProcedure;
  windowClass.hInstance = GetModuleHandle(nullptr);;
  windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  windowClass.lpszClassName = L"MyWindowClassName";
  RegisterClassEx(&windowClass);

  m_hwnd = CreateWindowEx(
    0
    , L"MyWindowClassName"
    , L"My Window"
    , WS_POPUP
    , CW_USEDEFAULT
    , 0
    , CW_USEDEFAULT
    , 0
    , nullptr
    , nullptr
    , windowClass.hInstance
    , this
    );

  if (!m_hwnd)
  {
    return false;
  }

  m_cursorArrow = LoadCursor(NULL, IDC_ARROW);
  m_cursorClick = LoadCursor(NULL, IDC_HAND);
  m_cursorResize[RESIZE_NONE] = LoadCursor(NULL, IDC_CROSS);
  m_cursorResize[RESIZE_MID] = LoadCursor(NULL, IDC_SIZEALL);
  m_cursorResize[RESIZE_TOP] = m_cursorResize[RESIZE_BTM] = LoadCursor(NULL, IDC_SIZENS);
  m_cursorResize[RESIZE_L] = m_cursorResize[RESIZE_R] = LoadCursor(NULL, IDC_SIZEWE);
  m_cursorResize[RESIZE_TOP_L] = m_cursorResize[RESIZE_BTM_R] = LoadCursor(NULL, IDC_SIZENWSE);
  m_cursorResize[RESIZE_TOP_R] = m_cursorResize[RESIZE_BTM_L] = LoadCursor(NULL, IDC_SIZENESW);

  m_font = CreateFontW(
    -UI_FONT_SIZE
    , 0
    , 0
    , 0
    , FW_NORMAL
    , FALSE
    , FALSE
    , FALSE
    , DEFAULT_CHARSET
    , OUT_DEFAULT_PRECIS
    , CLIP_DEFAULT_PRECIS
    , CLEARTYPE_QUALITY
    , DEFAULT_PITCH
    , UI_FONT.data());
  assert(m_font);

  m_fontBold = CreateFontW(
    -UI_FONT_SIZE
    , 0
    , 0
    , 0
    , FW_BOLD
    , FALSE
    , FALSE
    , FALSE
    , DEFAULT_CHARSET
    , OUT_DEFAULT_PRECIS
    , CLIP_DEFAULT_PRECIS
    , CLEARTYPE_QUALITY
    , DEFAULT_PITCH
    , UI_FONT.data());
  assert(m_fontBold);

  return true;
}

std::tuple<int, int, int, int> AreaSelector::run()
{
  ::ShowWindow(m_hwnd, SW_SHOW);
  ::UpdateWindow(m_hwnd);

  this->rectangleInit();

  MSG msg{0};
  while (::GetMessage(&msg, nullptr, 0, 0))
  {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }

  int X0 = std::min(m_rectSelection[0].x, m_rectSelection[1].x);
  int Y0 = std::min(m_rectSelection[0].y, m_rectSelection[1].y);
  int X1 = std::max(m_rectSelection[0].x, m_rectSelection[1].x);
  int Y1 = std::max(m_rectSelection[0].y, m_rectSelection[1].y);

  return std::make_tuple(X0, Y0, X1, Y1);
}

LRESULT CALLBACK AreaSelector::StaticWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  AreaSelector* pThis = nullptr;
  if (message == WM_NCCREATE)
  {
    CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
    pThis = static_cast<AreaSelector*>(pCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
  }
  else
  {
    pThis = reinterpret_cast<AreaSelector*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  if (pThis)
  {
    return pThis->privateWindowProcedure(hwnd, message, wParam, lParam);
  }

  // Handle any messages the switch statement didn't.
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void AreaSelector::rectangleInit()
{
  POINT Mouse;
  GetCursorPos(&Mouse);

  HMONITOR Monitor = MonitorFromPoint(Mouse, MONITOR_DEFAULTTONULL);
  if (Monitor == NULL)
  {
    MessageBox(nullptr, L"Unknown monitor!", L"Unknown monitor!", MB_OK);
    return;
  }

  MONITORINFOEXW Info{0};
  Info.cbSize = sizeof(Info);
  GetMonitorInfoW(Monitor, (LPMONITORINFO)&Info);

  HDC DeviceContext = CreateDCW(L"DISPLAY", Info.szDevice, NULL, NULL);
  if (DeviceContext == NULL)
  {
    MessageBox(nullptr, L"Error getting HDC of monitor!", L"ERROR", MB_OK);
    return;
  }

  DWORD Width = Info.rcMonitor.right - Info.rcMonitor.left;
  DWORD Height = Info.rcMonitor.bottom - Info.rcMonitor.top;

  // capture image from desktop
  HDC MemoryContext = CreateCompatibleDC(DeviceContext);
  assert(MemoryContext);

  HBITMAP MemoryBitmap = CreateCompatibleBitmap(DeviceContext, Width, Height);
  assert(MemoryBitmap);

  SelectObject(MemoryContext, MemoryBitmap);
  BitBlt(MemoryContext, 0, 0, Width, Height, DeviceContext, 0, 0, SRCCOPY);

  // prepare darkened image by doing alpha blend

  HDC MemoryDarkContext = CreateCompatibleDC(DeviceContext);
  assert(MemoryDarkContext);

  HBITMAP MemoryDarkBitmap = CreateCompatibleBitmap(DeviceContext, Width, Height);
  assert(MemoryDarkBitmap);

  BLENDFUNCTION Blend =
  {
    .BlendOp = AC_SRC_OVER,
    .SourceConstantAlpha = 0x40,
  };

  SelectObject(MemoryDarkContext, MemoryDarkBitmap);
  AlphaBlend(MemoryDarkContext, 0, 0, Width, Height, MemoryContext, 0, 0, Width, Height, Blend);

  // done
  DeleteDC(DeviceContext);

  m_rectMonitor = Monitor;
  m_rectContext = MemoryContext;
  m_rectDarkContext = MemoryDarkContext;
  m_rectBitmap = MemoryBitmap;
  m_rectDarkBitmap = MemoryDarkBitmap;
  m_rectWidth = Width;
  m_rectHeight = Height;
  m_rectSelected = FALSE;
  m_rectResize = RESIZE_NONE;
  m_rectSetSize[0] = m_rectSetSize[1] = 0;
  m_rectSetSizeClick = FALSE;

  SetCursor(m_cursorResize[RESIZE_NONE]);
  SetWindowPos(m_hwnd, HWND_TOPMOST, Info.rcMonitor.left, Info.rcMonitor.top, Width, Height, SWP_SHOWWINDOW);
  SetForegroundWindow(m_hwnd);
  InvalidateRect(m_hwnd, NULL, FALSE);
}

void AreaSelector::rectangleInitDone()
{
  ShowWindow(m_hwnd, SW_HIDE);
  SetCursor(m_cursorArrow);
  ReleaseCapture();

  if (m_rectContext)
  {
    DeleteDC(m_rectContext);
    m_rectContext = nullptr;

    DeleteObject(m_rectBitmap);
    m_rectBitmap = nullptr;

    DeleteDC(m_rectDarkContext);
    m_rectDarkContext = nullptr;

    DeleteObject(m_rectDarkBitmap);
    m_rectDarkBitmap = nullptr;
  }
}

int AreaSelector::getPointResize(int X, int Y)
{
  int BorderX = GetSystemMetrics(SM_CXSIZEFRAME);
  int BorderY = GetSystemMetrics(SM_CYSIZEFRAME);

  int X0 = std::min(m_rectSelection[0].x, m_rectSelection[1].x);
  int Y0 = std::min(m_rectSelection[0].y, m_rectSelection[1].y);
  int X1 = std::max(m_rectSelection[0].x, m_rectSelection[1].x);
  int Y1 = std::max(m_rectSelection[0].y, m_rectSelection[1].y);

  POINT P = { X, Y };

  RECT TL = { X0 - BorderX, Y0 - BorderY, X0 + BorderX, Y0 + BorderY };
  if (PtInRect(&TL, P)) return RESIZE_TOP_L;

  RECT TR = { X1 - BorderX, Y0 - BorderY, X1 + BorderX, Y0 + BorderY };
  if (PtInRect(&TR, P)) return RESIZE_TOP_R;

  RECT BL = { X0 - BorderX, Y1 - BorderY, X0 + BorderX, Y1 + BorderY };
  if (PtInRect(&BL, P)) return RESIZE_BTM_L;

  RECT BR = { X1 - BorderX, Y1 - BorderY, X1 + BorderX, Y1 + BorderY };
  if (PtInRect(&BR, P)) return RESIZE_BTM_R;

  RECT T = { X0, Y0 - BorderY, X1, Y0 + BorderY };
  if (PtInRect(&T, P)) return RESIZE_TOP;

  RECT B = { X0, Y1 - BorderY, X1, Y1 + BorderY };
  if (PtInRect(&B, P)) return RESIZE_BTM;

  RECT L = { X0 - BorderX, Y0, X0 + BorderX, Y1 };
  if (PtInRect(&L, P)) return RESIZE_L;

  RECT R = { X1 - BorderX, Y0, X1 + BorderX, Y1 };
  if (PtInRect(&R, P)) return RESIZE_R;

  RECT M = { X0, Y0, X1, Y1 };
  if (PtInRect(&M, P)) return RESIZE_MID;

  return RESIZE_NONE;
}

void AreaSelector::adjustRectSizeMultipleOf2(int Adjust, int Ref)
{
  int W = m_rectSelection[Ref].x - m_rectSelection[Adjust].x;
  W = (W + (W > 0)) & ~1;
  m_rectSelection[Adjust].x = m_rectSelection[Ref].x - W;

  int H = m_rectSelection[Ref].y - m_rectSelection[Adjust].y;
  H = (H + (H > 0)) & ~1;
  m_rectSelection[Adjust].y = m_rectSelection[Ref].y - H;
}

LRESULT AreaSelector::privateWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_KEYDOWN:
    {
      if (m_rectContext)
      {
        if (wParam == VK_ESCAPE)
        {
          rectangleInitDone();
          PostQuitMessage(0);
          break;
        }
      }
    }
    break;

    case WM_LBUTTONDOWN:
    {
      if (m_rectContext)
      {
        if (m_rectSetSize[0])
        {
          m_rectSetSizeClick = TRUE;
          m_rectSelection[1].x = m_rectSelection[0].x + m_rectSetSize[0];
          m_rectSelection[1].y = m_rectSelection[0].y + m_rectSetSize[1];
          InvalidateRect(hwnd, NULL, FALSE);
        }
        else
        {
          int X = GET_X_LPARAM(lParam);
          int Y = GET_Y_LPARAM(lParam);

          int Resize = m_rectSelected ? this->getPointResize(X, Y) : RESIZE_NONE;
          if (Resize == RESIZE_NONE)
          {
            // inital rectangle will be empty
            m_rectSelection[0].x = m_rectSelection[1].x = X;
            m_rectSelection[0].y = m_rectSelection[1].y = Y;
            m_rectSelected = FALSE;

            InvalidateRect(hwnd, NULL, FALSE);
          }
          else
          {
            // resizing direction
            m_rectMousePos.x = X;
            m_rectMousePos.y = Y;
          }
          m_rectResize = Resize;
          SetCapture(hwnd);
        }
      }
    }
    break;

    case WM_LBUTTONUP:
    {
      if (m_rectContext)
      {
        if (m_rectSetSizeClick)
        {
          m_rectSetSizeClick = FALSE;
        }
        else
        {
          if (m_rectSelected)
          {
            // fix the selected rectangle coordinates, so next resizing starts on the correct side
            int X0 = std::min(m_rectSelection[0].x, m_rectSelection[1].x);
            int Y0 = std::min(m_rectSelection[0].y, m_rectSelection[1].y);
            int X1 = std::max(m_rectSelection[0].x, m_rectSelection[1].x);
            int Y1 = std::max(m_rectSelection[0].y, m_rectSelection[1].y);
            m_rectSelection[0].x = X0;
            m_rectSelection[0].y = Y0;
            m_rectSelection[1].x = X1;
            m_rectSelection[1].y = Y1;
          }
          ReleaseCapture();
        }
      }
    }
    break;

    case WM_MOUSEMOVE:
    {
      if (m_rectContext)
      {
        int X = GET_X_LPARAM(lParam);
        int Y = GET_Y_LPARAM(lParam);

        if (m_rectSetSize[0])
        {
          SetCursor(m_cursorClick);
          InvalidateRect(hwnd, NULL, FALSE);
        }
        else if (m_rectSetSizeClick)
        {
          InvalidateRect(hwnd, NULL, FALSE);
        }
        else if (wParam & MK_LBUTTON)
        {
          BOOL Update = FALSE;

          if (m_rectResize == RESIZE_TOP_L || m_rectResize == RESIZE_L || m_rectResize == RESIZE_BTM_L)
          {
            // left moved
            m_rectSelection[0].x = X;
            this->adjustRectSizeMultipleOf2(0, 1);
            Update = TRUE;
          }
          else if (m_rectResize == RESIZE_TOP_R || m_rectResize == RESIZE_R || m_rectResize == RESIZE_BTM_R)
          {
            // right moved
            m_rectSelection[1].x = X;
            this->adjustRectSizeMultipleOf2(1, 0);
            Update = TRUE;
          }

          if (m_rectResize == RESIZE_TOP_L || m_rectResize == RESIZE_TOP || m_rectResize == RESIZE_TOP_R)
          {
            // top moved
            m_rectSelection[0].y = Y;
            this->adjustRectSizeMultipleOf2(0, 1);
            Update = TRUE;
          }
          else if (m_rectResize == RESIZE_BTM_L || m_rectResize == RESIZE_BTM || m_rectResize == RESIZE_BTM_R)
          {
            // bottom moved
            m_rectSelection[1].y = Y;
            this->adjustRectSizeMultipleOf2(1, 0);
            Update = TRUE;
          }

          if (m_rectResize == RESIZE_MID)
          {
            // if moving whole rectangle update both
            int DX = X - m_rectMousePos.x;
            int DY = Y - m_rectMousePos.y;
            m_rectMousePos.x = X;
            m_rectMousePos.y = Y;

            m_rectSelection[0].x += DX;
            m_rectSelection[0].y += DY;
            m_rectSelection[1].x += DX;
            m_rectSelection[1].y += DY;

            Update = TRUE;
          }
          else if (m_rectResize == RESIZE_NONE)
          {
            // no resize means we're selecting initial rectangle
            m_rectSelection[1].x = X;
            m_rectSelection[1].y = Y;
            this->adjustRectSizeMultipleOf2(1, 0);
            if (m_rectSelection[0].x != m_rectSelection[1].x && m_rectSelection[0].y != m_rectSelection[1].y)
            {
              // when we have non-zero size rectangle, we're good with initial stage
              m_rectSelected = TRUE;
              Update = TRUE;
            }
          }

          if (Update)
          {
            InvalidateRect(hwnd, NULL, FALSE);
          }
        }
        else
        {
          int Resize = m_rectSelected ? this->getPointResize(X, Y) : RESIZE_NONE;
          SetCursor(m_cursorResize[Resize]);

          if (Resize == RESIZE_NONE)
          {
            // in case hovering over resize text
            InvalidateRect(hwnd, NULL, FALSE);
          }
        }
      }
    }
    break;

    case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      HDC PaintContext = BeginPaint(hwnd, &Paint);

      HDC Context;
      HPAINTBUFFER BufferedPaint = BeginBufferedPaint(PaintContext, &Paint.rcPaint, BPBF_COMPATIBLEBITMAP, NULL, &Context);
      if (BufferedPaint)
      {
        if (m_rectContext)
        {
          {
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int W = Paint.rcPaint.right - Paint.rcPaint.left;
            int H = Paint.rcPaint.bottom - Paint.rcPaint.top;

            // draw darkened screenshot
            BitBlt(Context, X, Y, W, H, m_rectDarkContext, X, Y, SRCCOPY);
          }

          if (m_rectSelected)
          {
            // draw selected rectangle
            int X0 = std::min(m_rectSelection[0].x, m_rectSelection[1].x);
            int Y0 = std::min(m_rectSelection[0].y, m_rectSelection[1].y);
            int X1 = std::max(m_rectSelection[0].x, m_rectSelection[1].x);
            int Y1 = std::max(m_rectSelection[0].y, m_rectSelection[1].y);
            BitBlt(Context, X0, Y0, X1 - X0, Y1 - Y0, m_rectContext, X0, Y0, SRCCOPY);

            RECT Rect = { X0 - 1, Y0 - 1, X1 + 1, Y1 + 1 };
            FrameRect(Context, &Rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

            WCHAR Text[128];
            int TextLength = StrFormat(Text, L"%d x %d", X1 - X0, Y1 - Y0);

            SelectObject(Context, m_fontBold);
            SetTextAlign(Context, TA_TOP | TA_RIGHT);
            SetTextColor(Context, RGB(255, 255, 255));
            SetBkMode(Context, TRANSPARENT);
            ExtTextOutW(Context, X1, Y1, 0, NULL, Text, TextLength, NULL);

            SelectObject(Context, m_fontBold);
            SetTextAlign(Context, TA_BOTTOM | TA_LEFT);
            SetTextColor(Context, RGB(255, 255, 255));

            const WCHAR TextResize[] = L"Resize:  ";

            SIZE Size;
            GetTextExtentPoint32W(Context, TextResize, _countof(TextResize) - 1, &Size);
            ExtTextOutW(Context, X0, Y0, 0, NULL, TextResize, _countof(TextResize) - 1, NULL);

            int X = X0;
            SelectObject(Context, m_font);

            POINT CursorPos;
            GetCursorPos(&CursorPos);
            ScreenToClient(hwnd, &CursorPos);

            m_rectSetSize[0] = m_rectSetSize[1] = 0;

            int Sizes[][2] = { { 800, 600 }, { 1280, 720 }, { 1920, 1080 }, { 2560, 1440 } };
            for (int i = 0; i < _countof(Sizes); i++)
            {
              X += Size.cx;

              TextLength = StrFormat(Text, L"%dx%d  ", Sizes[i][0], Sizes[i][1]);
              GetTextExtentPoint32W(Context, Text, TextLength, &Size);

              RECT Rect = { X, Y0 - Size.cy, X + Size.cx, Y0 };
              BOOL Hovering = PtInRect(&Rect, CursorPos);
              SetTextColor(Context, Hovering ? RGB(255, 255, 255) : RGB(192, 192, 192));
              ExtTextOutW(Context, X, Y0, 0, NULL, Text, TextLength, NULL);

              if (Hovering)
              {
                m_rectSetSize[0] = Sizes[i][0];
                m_rectSetSize[1] = Sizes[i][1];
                SetCursor(m_cursorClick);
              }
            }
          }
          else
          {
            // draw initial message when no rectangle is selected
            SelectObject(Context, m_font);
            SelectObject(Context, GetStockObject(DC_PEN));
            SelectObject(Context, GetStockObject(DC_BRUSH));

            const WCHAR Line1[] = L"Select area with the mouse.";
            const WCHAR Line2[] = L"Press ESC key to cancel or done.";

            const WCHAR* Lines[] = { Line1, Line2 };
            const int LineLengths[] = { _countof(Line1) - 1, _countof(Line2) - 1 };
            int Widths[_countof(Lines)];
            int Height;

            int TotalWidth = 0;
            int TotalHeight = 0;
            for (int i = 0; i < _countof(Lines); i++)
            {
              SIZE Size;
              GetTextExtentPoint32W(Context, Lines[i], LineLengths[i], &Size);
              Widths[i] = Size.cx;
              Height = Size.cy;
              TotalWidth = std::max(TotalWidth, (int)Size.cx);
              TotalHeight += Size.cy;
            }
            TotalWidth += 2 * Height;
            TotalHeight += Height;

            int MsgX = (m_rectWidth - TotalWidth) / 2;
            int MsgY = (m_rectHeight - TotalHeight) / 2;

            SetDCPenColor(Context, RGB(255, 255, 255));
            SetDCBrushColor(Context, RGB(0, 0, 128));
            Rectangle(Context, MsgX, MsgY, MsgX + TotalWidth, MsgY + TotalHeight);

            SetTextAlign(Context, TA_TOP | TA_CENTER);
            SetTextColor(Context, RGB(255, 255, 0));
            SetBkMode(Context, TRANSPARENT);
            int Y = MsgY + Height / 2;
            int X = m_rectWidth / 2;
            for (int i = 0; i < _countof(Lines); i++)
            {
              ExtTextOutW(Context, X, Y, 0, NULL, Lines[i], LineLengths[i], NULL);
              Y += Height;
            }
          }
        }
        else
        {
          RECT Rect;
          GetClientRect(hwnd, &Rect);

          HBRUSH BorderBrush = CreateSolidBrush(RGB(255, 255, 0));
          assert(BorderBrush);
          FillRect(Context, &Rect, BorderBrush);
          DeleteObject(BorderBrush);

          Rect.left += RECT_BORDER;
          Rect.top += RECT_BORDER;
          Rect.right -= RECT_BORDER;
          Rect.bottom -= RECT_BORDER;

          HBRUSH ColorKeyBrush = CreateSolidBrush(RGB(255, 0, 255));
          assert(ColorKeyBrush);
          FillRect(Context, &Rect, ColorKeyBrush);
          DeleteObject(ColorKeyBrush);

          FrameRect(Context, &Rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }
        EndBufferedPaint(BufferedPaint, TRUE);
      }
      EndPaint(hwnd, &Paint);
      return 0;
    }
    break;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
  return 0;
}


