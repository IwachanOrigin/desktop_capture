
#ifndef CAPTURE_HELPER_H_
#define CAPTURE_HELPER_H_

#include "pch.h"
#include <windows.graphics.capture.interop.h>

namespace winrt
{
using namespace Windows::Graphics;
using namespace Windows::Graphics::Capture;
using namespace Windows::UI::Composition;
using namespace Windows::Graphics::Capture;
}

namespace capturehelper
{

inline auto CreateCaptureItemForWindows(HWND hwnd)
{
  auto interop_factory = winrt::get_activation_factory<winrt::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
  winrt::GraphicsCaptureItem item = {nullptr};
  winrt::check_hresult(interop_factory->CreateForWindow(hwnd, winrt::guid_of<winrt::IGraphicsCaptureItem>(), winrt::put_abi(item)));
  return item;
}

inline auto CreateCaptureItemForMonitor(HMONITOR hmon)
{
  auto interop_factory = winrt::get_activation_factory<winrt::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
  winrt::GraphicsCaptureItem item = {nullptr};
  winrt::check_hresult(interop_factory->CreateForMonitor(hmon, winrt::guid_of<winrt::IGraphicsCaptureItem>(), winrt::put_abi(item)));
  return item;
}

} // capturehelper

#endif // CAPTURE_HELPER_H_
