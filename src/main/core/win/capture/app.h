
#ifndef APP_H_
#define APP_H_

#include "pch.h"
#include "hwndcapture.h"
#include "renderercallback.h"

namespace winrt
{
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::System;
}

namespace directx
{
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
}

namespace core
{

class App
{
public:
  explicit App(winrt::ContainerVisual root, winrt::GraphicsCapturePicker capturePicker, winrt::FileSavePicker savePicker);
  ~App() = default;

  winrt::GraphicsCaptureItem tryStartCaptureFromWindowHandle(HWND hwnd);
  winrt::GraphicsCaptureItem tryStartCaptureFromMonitorHandle(HMONITOR hmon);
  winrt::IAsyncOperation<winrt::GraphicsCaptureItem> startCaptureWithPickerAsync();
  //winrt::IAsyncOperation<winrt::StorageFile> takeSnapshotAsync();
  directx::DirectXPixelFormat pixelFormat() const { return m_pixelFormat; }
  void setPixelFormat(directx::DirectXPixelFormat pixelFormat);

  bool isCursorEnabled();
  void setCursorEnabled(const bool& value);
  bool isBorderRequired();
  winrt::fire_and_forget setBorderRequired(const bool& value);
  bool isCaptureStatus() const { return m_captureStatus; }

  int frameCount() const;
  void stopCapture();

  void setRendererCallbackFunc(const RendererCallback& callback) { m_rendererCallback = callback; }

private:
  void startCaptureFromItem(winrt::GraphicsCaptureItem item);

  winrt::DispatcherQueue m_mainThread{ nullptr };
  winrt::Compositor m_compositor{ nullptr };
  winrt::ContainerVisual m_root{ nullptr };
  winrt::SpriteVisual m_content{ nullptr };
  winrt::CompositionSurfaceBrush m_brush{ nullptr };

  winrt::GraphicsCapturePicker m_capturePicker{ nullptr };
  winrt::FileSavePicker m_savePicker{ nullptr };

  directx::IDirect3DDevice m_device{ nullptr };
  directx::DirectXPixelFormat m_pixelFormat = directx::DirectXPixelFormat::R8G8B8A8UIntNormalized;
  std::unique_ptr<core::HwndCapture> m_capture{ nullptr };

  bool m_captureStatus = false;
  RendererCallback m_rendererCallback = nullptr;
};

}

#endif // APP_H_
