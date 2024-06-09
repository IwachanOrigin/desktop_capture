
#include "pch.h"
#include "SimpleMath.h"
#include "dx11baserenderer.h"

using namespace DirectX;
using namespace renderer;

bool DX11BaseRenderer::init(
  DeviceResources& deviceResources
  , std::shared_ptr<renderer::NVScaler> nvScaler
  , std::shared_ptr<renderer::NVSharpen> nvSharpen
  , const uint32_t& inputTextureWidth
  , const uint32_t& inputTextureHeight
  , const uint32_t& upscaleTextureWidth
  , const uint32_t& upscaleTextureHeight
  )
{
  m_deviceResources = deviceResources;
  m_nvScaler = nvScaler;
  m_nvSharpen = nvSharpen;
  m_inputTextureWidth = inputTextureWidth;
  m_inputTextureHeight = inputTextureHeight;
  m_upscaleTextureWidth = upscaleTextureWidth;
  m_upscaleTextureHeight = upscaleTextureHeight;
  auto result = this->createPipeline();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create pipeline.", L"Error", MB_OK);
    return false;
  }

  result = this->createMesh();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create quad mesh.", L"Error", MB_OK);
    return false;
  }

  result = this->createInputTexture();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create input texture.", L"Error", MB_OK);
    return false;
  }

  result = this->createInputSRV();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create input shader resource view.", L"Error", MB_OK);
    return false;
  }

  result = this->createLinearClampSampler();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create linear clamp sampler.", L"Error", MB_OK);
    return false;
  }

  result = this->createUpscaledTexture();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create upscaled texture.", L"Error", MB_OK);
    return false;
  }

  result = this->createUAV();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create unordered access view.", L"Error", MB_OK);
    return false;
  }

  result = this->createUpscaledSRV();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create upscaled shader resource view.", L"Error", MB_OK);
    return false;
  }

  return true;
}

bool DX11BaseRenderer::render()
{
  if (!m_deviceResources.context())
  {
    return false;
  }

  // If not upscale, display to InputTexture
  if (!m_nvScaler && !m_nvSharpen)
  {
    // Set the shader's
    m_pipeline.activate();
    if (m_inputSRV)
    {
      m_deviceResources.context()->PSSetShaderResources(0, 1, m_inputSRV.GetAddressOf());
    }
    m_quad.activateAndRender();
  }
  else
  {
    // Set the shader's
    m_pipeline.activate();
    if (m_upscaledSRV)
    {
      m_deviceResources.context()->PSSetShaderResources(0, 1, m_upscaledSRV.GetAddressOf());
    }
    m_quad.activateAndRender();
  }
  // Present swapchain
  if (m_deviceResources.swapchain())
  {
    auto hr = m_deviceResources.present(1, 0);
    if (FAILED(hr))
    {
      MessageBoxW(nullptr, L"Failed to present in Swap Chain.", L"Error", MB_OK);
      return false;
    }
  }
  return true;
}

bool DX11BaseRenderer::createPipeline()
{
  D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  if (!m_pipeline.create(layout, ARRAYSIZE(layout), m_deviceResources.device(), m_deviceResources.context()))
  {
    MessageBoxW(nullptr, L"Failed to create Pipeline.", L"Error", MB_OK);
    return false;
  }

  return true;
}

bool DX11BaseRenderer::createMesh()
{
  struct Vertex {
    float x, y, z;
    float u, v;
  };

  {
    /* texture vertex(x, y, z)
     *              ^
     *              |
     *   -1,1       |        1,1
     *              |
     *              |
     *   ----------0,0------------->
     *              |
     *              |
     *  -1,-1       |        1,-1
     *              |
     *              |
     *
     */

    /* texture color(u, v)
     *
     *           U
     *   0,0-------------> 1,0
     *    |
     *    |
     *    |
     *  V |
     *    |
     *    |
     *    v
     *   1,0               1,1
     */
    SimpleMath::Vector4 white(1, 1, 1, 1);
    std::vector<Vertex> vtxs = {
      { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
      { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f },
      {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
      { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f },
      {  1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
      {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
    };
    if (!m_quad.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(Vertex), renderer::eTopology::TRIANGLE_LIST, m_deviceResources.device(), m_deviceResources.context()))
    {
      MessageBoxW(nullptr, L"Failed to create Quad Mesh.", L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

bool DX11BaseRenderer::createInputTexture()
{
  auto ret = m_deviceResources.createTexture(m_inputTexture, m_inputTextureWidth, m_inputTextureHeight, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_USAGE_DYNAMIC);
  return ret;
}

bool DX11BaseRenderer::createUpscaledTexture()
{
  //auto ret = m_deviceResources.createTexture(m_upscaledTexture, m_upscaleTextureWidth, m_upscaleTextureHeight, DXGI_FORMAT_B8G8R8A8_UNORM, D3D11_USAGE_DYNAMIC);
  auto ret = m_deviceResources.createTexture(m_upscaledTexture, m_upscaleTextureWidth, m_upscaleTextureHeight, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_USAGE_DEFAULT);
  return ret;
}

bool DX11BaseRenderer::createInputSRV()
{
  auto ret = m_deviceResources.createSRV(m_inputTexture.Get(), m_inputSRV, DXGI_FORMAT_R8G8B8A8_UNORM);
  return ret;
}

bool DX11BaseRenderer::createUpscaledSRV()
{
  auto ret = m_deviceResources.createSRV(m_upscaledTexture.Get(), m_upscaledSRV, DXGI_FORMAT_R8G8B8A8_UNORM);
  return ret;
}

bool DX11BaseRenderer::createUAV()
{
  auto ret = m_deviceResources.createUAV(m_upscaledTexture.Get(), DXGI_FORMAT_R8G8B8A8_UNORM, m_upscaledUAV);
  return ret;
}

bool DX11BaseRenderer::createLinearClampSampler()
{
  auto ret = m_deviceResources.createLinearClampSampler(m_samplerClampLinear);
  return ret;
}

