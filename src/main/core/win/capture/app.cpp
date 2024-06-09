
#include "app.h"
#include "d3dHelpers.h"
#include "capturehelper.h"
#include "direct3d11.interop.h"

#include <iostream>

using namespace core;

App::App(winrt::ContainerVisual root, winrt::GraphicsCapturePicker capturePicker, winrt::FileSavePicker savePicker)
  : m_capturePicker(capturePicker)
  , m_savePicker(savePicker)
{
  m_mainThread = winrt::DispatcherQueue::GetForCurrentThread();
  WINRT_VERIFY(m_mainThread != nullptr);

  m_compositor = root.Compositor();
  m_root = m_compositor.CreateContainerVisual();
  m_content = m_compositor.CreateSpriteVisual();
  m_brush = m_compositor.CreateSurfaceBrush();

  m_root.RelativeSizeAdjustment({1, 1});
  root.Children().InsertAtTop(m_root);

  m_content.AnchorPoint({0.5f, 0.5f});
  m_content.RelativeOffsetAdjustment({0.5f, 0.5f, 0});
  m_content.RelativeSizeAdjustment({1, 1});
  m_content.Size({-80, -80});
  m_content.Brush(m_brush);
  m_brush.HorizontalAlignmentRatio(0.5f);
  m_brush.VerticalAlignmentRatio(0.5f);
  m_brush.Stretch(winrt::CompositionStretch::Uniform);
  auto shadow = m_compositor.CreateDropShadow();
  shadow.Mask(m_brush);
  m_content.Shadow(shadow);
  m_root.Children().InsertAtTop(m_content);

  auto d3dDevice = common::uwp::CreateD3DDevice();
  auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
  m_device = CreateDirect3DDevice(dxgiDevice.get());
}

winrt::GraphicsCaptureItem App::tryStartCaptureFromWindowHandle(HWND hwnd)
{
  winrt::GraphicsCaptureItem item{nullptr};
  try
  {
    item = capturehelper::CreateCaptureItemForWindows(hwnd);
    this->startCaptureFromItem(item);
    m_captureStatus = true;
  }
  catch (const winrt::hresult_error& error)
  {
    std::wcerr << L"error : " << error.message().c_str() << std::endl;
  }
  return item;
}

winrt::GraphicsCaptureItem App::tryStartCaptureFromMonitorHandle(HMONITOR hmon)
{
  winrt::GraphicsCaptureItem item{nullptr};
  try
  {
    item = capturehelper::CreateCaptureItemForMonitor(hmon);
    this->startCaptureFromItem(item);
    m_captureStatus = true;
  }
  catch (const winrt::hresult_error& error)
  {
    std::wcerr << L"error : " << error.message().c_str() << std::endl;
  }
  return item;
}

winrt::IAsyncOperation<winrt::GraphicsCaptureItem> App::startCaptureWithPickerAsync()
{
  auto item = co_await m_capturePicker.PickSingleItemAsync();
  co_return item;
}


void App::setPixelFormat(directx::DirectXPixelFormat pixelFormat)
{
  m_pixelFormat = pixelFormat;
  if (m_capture)
  {
    // set pixel format to capture.
  }
}

bool App::isCursorEnabled()
{
  if (!m_capture)
  {
    return m_capture->isCursorEnabled();
  }
  return false;
}

void App::setCursorEnabled(const bool& value)
{
  if (!m_capture)
  {
    m_capture->setCursorEnabled(value);
  }
}

bool App::isBorderRequired()
{
  if (!m_capture)
  {
    return m_capture->isBorderRequired();
  }
  return false;
}

winrt::fire_and_forget App::setBorderRequired(const bool& value)
{
  if (!m_capture)
  {
    // Even if the user or system policy denies access, it's
    // still safe to set the IsBorderRequired property. In the
    // event that the policy changes, the property will be honored.
    auto ignored = co_await winrt::GraphicsCaptureAccess::RequestAccessAsync(winrt::GraphicsCaptureAccessKind::Borderless);
    m_capture->setBorderRequired(value);
  }
}

int App::frameCount() const
{
  if (m_capture)
  {
    return m_capture->frameCount();
  }
  return -1;
}

void App::stopCapture()
{
  if (m_capture)
  {
    m_capture->close();
    m_capture = nullptr;
    m_brush.Surface(nullptr);
  }
  m_captureStatus = false;
}

void App::startCaptureFromItem(winrt::GraphicsCaptureItem item)
{
  m_capture = std::make_unique<HwndCapture>(m_device, item, m_pixelFormat);
  if (m_rendererCallback)
  {
    m_capture->setRendererCallbackFunc(m_rendererCallback);
  }
  assert(m_rendererCallback);
  auto surface = m_capture->createSurface(m_compositor);
  m_brush.Surface(surface);
  m_capture->startCapture();
}


