
#include "pch.h"
#include "dx11rgb32renderer.h"

using namespace DirectX;
using namespace renderer;

DX11RGB32Renderer::DX11RGB32Renderer()
  : DX11BaseRenderer()
{
}

DX11RGB32Renderer::~DX11RGB32Renderer()
{
}

bool DX11RGB32Renderer::updateTexture(const uint8_t* newData, const size_t dataSize)
{
  if (!newData)
  {
    return false;
  }

  if (!m_deviceResources.context())
  {
    return false;
  }

  // 
  D3D11_MAPPED_SUBRESOURCE ms{};
  HRESULT hr = m_deviceResources.context()->Map(m_inputTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
  if (FAILED(hr))
  {
    std::cerr << "Failed to mapping m_texture." << std::endl;
    return false;
  }

  const uint8_t* src = newData;
  uint8_t* dst = (uint8_t*)ms.pData;

  std::memcpy(dst, src, dataSize);

  m_deviceResources.context()->Unmap(m_inputTexture.Get(), 0);

  // During upscaling, the CopySubresourceRegion and OMSetRenderTargets functions are executed within the updateTexture function.
  // Calling the clearRenderTargetView function after the OMSetRenderTargets function is executed clears the texture on the buffer.
  // Therefore, the buffer is cleared here only if upscaling is not performed.
  // Clear back buffer
  float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // red,green,blue,alpha
  m_deviceResources.clearRenderTargetView(clearColor);

  if (m_scaledAreaConsBuffer)
  {
    m_deviceResources.context()->VSSetConstantBuffers(0, 1, m_scaledAreaConsBuffer.GetAddressOf());
    m_deviceResources.context()->PSSetConstantBuffers(0, 1, m_scaledAreaConsBuffer.GetAddressOf());
  }

  // Dispatch
  if (m_nvScaler)
  {
    // Calculate
    m_nvScaler->dispatch(m_inputSRV.GetAddressOf(), m_upscaledUAV.GetAddressOf());

#if 0
    // Copy to rendertarget from upsacledTexture
    // When drawing via shaders was failing, I used to write directly.
    D3D11_BOX srcRegion{0};
    srcRegion.left = 0;
    srcRegion.right = m_deviceResources.renderWidth();
    srcRegion.top = 0;
    srcRegion.bottom = m_deviceResources.renderHeight();
    srcRegion.front = 0;
    srcRegion.back = 1;
    m_deviceResources.context()->CopySubresourceRegion(m_deviceResources.renderTarget().Get(), 0, 0, 0, 0, m_upscaledTexture.Get(), 0, &srcRegion);
    m_deviceResources.context()->OMSetRenderTargets(1, m_deviceResources.renderTargetView().GetAddressOf(), nullptr);
#endif
    // Release UAV
    // When writing via shaders, UnorderedAccessView must be opened.
    ComPtr<ID3D11UnorderedAccessView> nullUAV = nullptr;
    m_deviceResources.context()->CSSetUnorderedAccessViews(0, 1, nullUAV.GetAddressOf(), nullptr);
    m_deviceResources.context()->OMSetRenderTargets(1, m_deviceResources.renderTargetView().GetAddressOf(), nullptr);
  }

  // Rendering  
  this->render();
  return true;
}

bool DX11RGB32Renderer::createScaledAreaCB(const int& scaledTop, const int& scaledLeft, const int& scaledRight, const int& scaledBottom, const int& textureWidth, const int& textureHeight)
{
  D3D11_BUFFER_DESC cbd{};
  cbd.Usage = D3D11_USAGE_DEFAULT;
  cbd.ByteWidth = sizeof(ConstantBufferTextureSize);
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.CPUAccessFlags = 0;

  // Ensure that the size of the constant buffer is a multiple of 16 bytes
  if (cbd.ByteWidth % 16 != 0)
  {
    cbd.ByteWidth = (cbd.ByteWidth + 15) / 16 * 16; // Padding for 16-byte alignment
  }
  auto hr = m_deviceResources.device()->CreateBuffer(&cbd, nullptr, m_scaledAreaConsBuffer.GetAddressOf());
  if (FAILED(hr))
  {
    return false;
  }

  ConstantBufferTextureSize cbTexSize{};
  cbTexSize.scaledTop = static_cast<float>(scaledTop);
  cbTexSize.scaledLeft = static_cast<float>(scaledLeft);
  cbTexSize.scaledRight = static_cast<float>(scaledRight);
  cbTexSize.scaledBottom = static_cast<float>(scaledBottom);
  cbTexSize.textureWidth = static_cast<float>(textureWidth);
  cbTexSize.textureHeight = static_cast<float>(textureHeight);

  m_deviceResources.context()->UpdateSubresource(m_scaledAreaConsBuffer.Get(), 0, nullptr, &cbTexSize, 0, 0);

  return true;
}


