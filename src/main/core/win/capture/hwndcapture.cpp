
#include "hwndcapture.h"
#include "d3dHelpers.h"
#include "direct3d11.interop.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace core;

HwndCapture::HwndCapture(const directx::Direct3D11::IDirect3DDevice& device, const winrt::GraphicsCaptureItem& item, const directx::DirectXPixelFormat pixelFormat)
  : m_item(item)
  , m_device(device)
  , m_pixelFormat(pixelFormat)
{
  // setup
  auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
  d3dDevice->GetImmediateContext(m_d3dContext.put());

  auto size = m_item.Size();
  m_swapChain = common::uwp::CreateDXGISwapChain(
    d3dDevice
    , static_cast<uint32_t>(size.Width)
    , static_cast<uint32_t>(size.Height)
    , static_cast<DXGI_FORMAT>(m_pixelFormat)
    , 2
    );

  // Create framepool, define pixel format (DXGI_FORMAT_R8G8B8A8_UNORM), and frame size. 
  m_framePool = winrt::Direct3D11CaptureFramePool::Create(
    m_device
    , m_pixelFormat
    , 2
    , size
    );
  m_session = m_framePool.CreateCaptureSession(m_item);
  m_lastSize = size;
  m_frameArrived = m_framePool.FrameArrived(winrt::auto_revoke, { this, &HwndCapture::onFrameArrived });
}

void HwndCapture::startCapture()
{
  this->checkClosed();
  m_session.StartCapture();
}

void HwndCapture::close()
{
  auto expected = false;
  if (m_closed.compare_exchange_strong(expected, true))
  {
    m_frameArrived.revoke();
    m_framePool.Close();
    m_session.Close();

    m_swapChain = nullptr;
    m_framePool = nullptr;
    m_session = nullptr;
    m_item = nullptr;
  }

  std::cout << "Finished capture." << std::endl;
}

winrt::ICompositionSurface HwndCapture::createSurface(const winrt::Compositor& compositor)
{
  this->checkClosed();
  return CreateCompositionSurfaceForSwapChain(compositor, m_swapChain.get());
}

void HwndCapture::setPixelFormat(directx::DirectXPixelFormat pixelFormat)
{
  this->checkClosed();
  auto newFormat = std::optional(pixelFormat);
}

void HwndCapture::onFrameArrived(const winrt::Direct3D11CaptureFramePool& sender, const winrt::IInspectable args)
{
  auto newSize = false;
  {
    auto frame = sender.TryGetNextFrame();
    auto frameContentSize = frame.ContentSize();

    if (frameContentSize.Width != m_lastSize.Width
      || frameContentSize.Height != m_lastSize.Height)
    {
      //
      newSize = true;
      m_lastSize = frameContentSize;
      m_swapChain->ResizeBuffers(
          2
        , static_cast<uint32_t>(m_lastSize.Width)
        , static_cast<uint32_t>(m_lastSize.Height)
        , static_cast<DXGI_FORMAT>(directx::DirectXPixelFormat::R8G8B8A8UIntNormalized)
        , 0);
    }

    {
      // The D3D11Texture2D generated here is GPU-specific.
      auto frameSurface = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());
      D3D11_TEXTURE2D_DESC frameSurfaceDesc = {};
      frameSurface->GetDesc(&frameSurfaceDesc);

      // Create staging desc.
      D3D11_TEXTURE2D_DESC stagingDesc = {};
      stagingDesc = frameSurfaceDesc;
      stagingDesc.Usage = D3D11_USAGE_STAGING;
      stagingDesc.BindFlags = 0;
      stagingDesc.MiscFlags = 0;
      stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

      auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
      winrt::com_ptr<ID3D11Texture2D> stagingBuffer;
      d3dDevice->CreateTexture2D(&stagingDesc, nullptr, stagingBuffer.put());
      m_d3dContext->CopyResource(stagingBuffer.get(), frameSurface.get());
      m_frameCount++;

      // Call callback renderer and send a stagingBuffer to renderer
      if (m_rendererCallback)
      {
        D3D11_MAPPED_SUBRESOURCE stagingMappedSubresource{};
        HRESULT hr = m_d3dContext->Map(stagingBuffer.get(), 0, D3D11_MAP_READ, 0, &stagingMappedSubresource);
        if (FAILED(hr))
        {
          std::cerr << "Failed Map stagingBuffer." << std::endl;
        }
        const uint8_t* srcSlice = { (uint8_t*)stagingMappedSubresource.pData };
        const int srcStride = {4 * m_lastSize.Width * m_lastSize.Height};
        auto result = m_rendererCallback(srcSlice, srcStride);
        m_d3dContext->Unmap(stagingBuffer.get(), 0);
        if (!result)
        {
          std::cerr << "Failed callback renderer." << std::endl;
        }
      }
    }
  }
}

