
#ifndef HWND_CAPTURE_H_
#define HWND_CAPTURE_H_

#include "pch.h"
#include "renderercallback.h"

namespace winrt
{
using namespace Windows::Foundation;
using namespace Windows::Foundation::Metadata;
using namespace Windows::Graphics;
using namespace Windows::Graphics::Capture;
using namespace Windows::Storage::Pickers;
using namespace Windows::UI::Composition;
}

namespace directx
{
using namespace winrt::Windows::Graphics::DirectX;
}

namespace core
{

class HwndCapture
{
public:
  explicit HwndCapture(const directx::Direct3D11::IDirect3DDevice& device, const winrt::GraphicsCaptureItem& item, const directx::DirectXPixelFormat pixelFormat);
  ~HwndCapture() { this->close(); }

  void startCapture();
  void close();

  winrt::ICompositionSurface createSurface(const winrt::Compositor& compositor);

  bool isCursorEnabled() { this->checkClosed(); return m_session.IsCursorCaptureEnabled(); }
  void setCursorEnabled(const bool& value) { this->checkClosed(); m_session.IsCursorCaptureEnabled(value); }
  bool isBorderRequired() { this->checkClosed(); return m_session.IsBorderRequired(); }
  void setBorderRequired(const bool& value) { this->checkClosed(); m_session.IsBorderRequired(value); }
  winrt::GraphicsCaptureItem captureItem() const { return m_item; }
  void setPixelFormat(directx::DirectXPixelFormat pixelFormat);

  int frameCount() const { return m_frameCount; }
  void setRendererCallbackFunc(const RendererCallback& callback) { m_rendererCallback = callback; }

private:
  winrt::GraphicsCaptureItem m_item{ nullptr };
  winrt::Direct3D11CaptureFramePool m_framePool{ nullptr };
  winrt::GraphicsCaptureSession m_session{ nullptr };
  winrt::SizeInt32 m_lastSize{ 0 };

  directx::Direct3D11::IDirect3DDevice m_device{ nullptr };
  winrt::com_ptr<IDXGISwapChain1> m_swapChain{ nullptr };
  winrt::com_ptr<ID3D11DeviceContext> m_d3dContext{ nullptr };
  directx::DirectXPixelFormat m_pixelFormat;

  std::atomic<std::optional<directx::DirectXPixelFormat>> m_pixelFormatUpdate;
  std::atomic<bool> m_closed = false;
  winrt::Direct3D11CaptureFramePool::FrameArrived_revoker m_frameArrived;

  int m_frameCount = 0;
  RendererCallback m_rendererCallback = nullptr;

  void onFrameArrived(const winrt::Direct3D11CaptureFramePool& sender, const winrt::IInspectable args);
  void checkClosed() { if (m_closed.load() == true) { throw winrt::hresult_error(RO_E_CLOSED); } }
};

} // core

#endif // HWND_CAPTURE_H_
