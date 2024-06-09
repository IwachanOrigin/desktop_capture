
#ifndef AREA_SELECTOR_H_
#define AREA_SELECTOR_H_

#include <Windows.h>
#include <Windowsx.h>
#include <string>
#include <tuple>

namespace common
{

constexpr int RESIZE_NONE  = 0;
constexpr int RESIZE_TOP_L = 1;
constexpr int RESIZE_TOP   = 2;
constexpr int RESIZE_TOP_R = 3;
constexpr int RESIZE_L     = 4;
constexpr int RESIZE_MID   = 5;
constexpr int RESIZE_R     = 6;
constexpr int RESIZE_BTM_L = 7;
constexpr int RESIZE_BTM   = 8;
constexpr int RESIZE_BTM_R = 9;

constexpr int RECT_BORDER = 2;

const std::wstring UI_FONT = L"Segoe UI";
constexpr int UI_FONT_SIZE = 16;

class AreaSelector
{
public:
  explicit AreaSelector() = default;
  ~AreaSelector() = default;

  bool init();
  std::tuple<int, int, int, int> run();

private:
  void adjustRectSizeMultipleOf2(int Adjust, int Ref);
  int getPointResize(int X, int Y);
  void rectangleInit();
  void rectangleInitDone();

  static LRESULT StaticWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT privateWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  HWND m_hwnd = nullptr;

  // When selecting rectangle to record
  HMONITOR m_rectMonitor = nullptr;
  HDC m_rectContext = nullptr;
  HDC m_rectDarkContext = nullptr;
  HBITMAP m_rectBitmap = nullptr;
  HBITMAP m_rectDarkBitmap = nullptr;
  DWORD m_rectWidth = 0;
  DWORD m_rectHeight = 0;
  BOOL m_rectSelected = FALSE;
  POINT m_rectSelection[2]{};
  POINT m_rectMousePos{};
  int m_rectResize{};
  int m_rectSetSize[2]{};
  BOOL m_rectSetSizeClick = FALSE;

  HCURSOR m_cursorArrow = nullptr;
  HCURSOR m_cursorClick = nullptr;
  HCURSOR m_cursorResize[10]{};

  HFONT m_font = nullptr;
  HFONT m_fontBold = nullptr;

};

} // common

#endif // AREA_SELECTOR_H_


