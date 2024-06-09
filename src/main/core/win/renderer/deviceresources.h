
#ifndef DEVICE_RESOURCES_H_
#define DEVICE_RESOURCES_H_

#include "pch.h"

using namespace Microsoft::WRL;

namespace renderer
{

class DeviceResources
{
public:
  explicit DeviceResources() = default;
  ~DeviceResources() = default;

  bool create(HWND hwnd, const uint32_t& swapChainWidth, const uint32_t& swapChainHeight, const uint32_t& fpsNum);
  bool resizeRenderTarget(const uint32_t& width, const uint32_t& height, const DXGI_FORMAT& format);
  void clearRenderTargetView(const float color[4]);
  auto present(const uint32_t& syncInterval, const uint32_t flags) { return m_swapchain->Present(syncInterval, flags); }
  ComPtr<ID3D11Device>& device() { return m_d3dDevice; }
  ComPtr<ID3D11DeviceContext>& context() { return m_immediateContext; }
  ComPtr<ID3D11Texture2D>& renderTarget() { return m_renderTarget; }
  ComPtr<ID3D11RenderTargetView>& renderTargetView() { return m_renderTargetView; }
  ComPtr<IDXGISwapChain>& swapchain() { return m_swapchain; }

  bool createTexture(
    ComPtr<ID3D11Texture2D>& resource
    , const uint32_t& width
    , const uint32_t& height
    , const DXGI_FORMAT& format
    , const D3D11_USAGE& heapType
    , const void* data = nullptr
    , const uint32_t& rowPitch = 0
    , const uint32_t& imageSize = 0
    );
  bool createSRV(ID3D11Resource* resource, ComPtr<ID3D11ShaderResourceView>& srv, const DXGI_FORMAT& format, const UINT& mipLevel = 1);
  bool createUAV(ID3D11Resource* resource, const DXGI_FORMAT& format, ComPtr<ID3D11UnorderedAccessView>& ppUAView);
  bool createLinearClampSampler(ComPtr<ID3D11SamplerState>& samplerState);
  bool createConstBuffer(void* data, const uint32_t& size, ComPtr<ID3D11Buffer>& ppBuffer);
  bool updateConstBuffer(void* data, const uint32_t& size, ComPtr<ID3D11Buffer>& ppBuffer);

  uint32_t renderWidth() const { return m_renderWidth; }
  uint32_t renderHeight() const { return m_renderHeight; }
  uint32_t swapChainWidth() const { return m_swapChainWidth; }
  uint32_t swapChainHeight() const { return m_swapChainHeight; }

  void getTextureData(ComPtr<ID3D11Texture2D>& texture, std::vector<uint8_t>& data, uint32_t& width, uint32_t& height, uint32_t& rowPitch);

private:
  bool initRenderTarget();

  ComPtr<ID3D11Device> m_d3dDevice = nullptr;
  ComPtr<ID3D11DeviceContext> m_immediateContext = nullptr;
  ComPtr<ID3D11Texture2D> m_renderTarget = nullptr;
  ComPtr<ID3D11RenderTargetView> m_renderTargetView = nullptr;
  ComPtr<IDXGISwapChain> m_swapchain = nullptr;

  uint32_t m_renderWidth = 0;
  uint32_t m_renderHeight = 0;
  uint32_t m_swapChainWidth = 0;
  uint32_t m_swapChainHeight = 0;
};

} // renderer

#endif // DEVICE_RESOURCES_H_

