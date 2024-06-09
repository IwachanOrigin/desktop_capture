
#include "deviceresources.h"

using namespace renderer;

bool DeviceResources::create(HWND hwnd, const uint32_t& swapChainWidth, const uint32_t& swapChainHeight, const uint32_t& fpsNum)
{
  if (!hwnd)
  {
    MessageBoxW(nullptr, L"hwnd is NULL.", L"Error", MB_OK);
    return false;
  }

  m_swapChainWidth = swapChainWidth;
  m_swapChainHeight = swapChainHeight;

  // Normally, the following method of determining the window size is not a problem.
  // However, the window size is so large that if the size is reduced for debugging, only a quarter of the window will be displayed, etc., so it is commented out now.
  //RECT rc{};
  //GetClientRect(hwnd, &rc);
  //m_renderWidth = rc.right - rc.left;
  //m_renderHeight = rc.bottom - rc.top;

  // Ignore window size.
  m_renderWidth = swapChainWidth;
  m_renderHeight = swapChainHeight;

  D3D_DRIVER_TYPE driverTypes[] = {
    D3D_DRIVER_TYPE_HARDWARE
    , D3D_DRIVER_TYPE_WARP
    , D3D_DRIVER_TYPE_REFERENCE
  };

  UINT numDriverTypes = ARRAYSIZE(driverTypes);

  DXGI_SWAP_CHAIN_DESC sd{};
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = m_swapChainWidth;            // Width of resolution
  sd.BufferDesc.Height = m_swapChainHeight;          // Height of resolution
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format
  sd.BufferDesc.RefreshRate.Numerator = fpsNum;      // Numerator of fps
  sd.BufferDesc.RefreshRate.Denominator = 1;         // Denominator of fps
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // Use of render target
  sd.SampleDesc.Count = 1;                           // Multisampling of sampling count
  sd.SampleDesc.Quality = 0;                         // Quality of multisampling
  sd.OutputWindow = hwnd;                            // Output window handl
  sd.Windowed = true;                                // Window mode
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Allow to change the mode of window and fullscreen

  D3D_FEATURE_LEVEL featureLevels[] = {
    D3D_FEATURE_LEVEL_11_1
    , D3D_FEATURE_LEVEL_11_0
    , D3D_FEATURE_LEVEL_10_1
    , D3D_FEATURE_LEVEL_10_0
  };

  UINT numFeatureLevels = ARRAYSIZE(featureLevels);

  D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

  HRESULT hr = E_FAIL;
  for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
  {
    driverType = driverTypes[driverTypeIndex];
    hr = D3D11CreateDeviceAndSwapChain(
      nullptr
      , driverType
      , nullptr
      , 0
      , featureLevels
      , numFeatureLevels
      , D3D11_SDK_VERSION
      , &sd
      , &m_swapchain
      , &m_d3dDevice
      , &featureLevel
      , &m_immediateContext);

    if (SUCCEEDED(hr))
    {
      break;
    }
  }

  if (FAILED(hr))
  {
    ::MessageBox(nullptr, L"Failed to create DirectX Device and SwapChain.", L"Error", MB_OK);
    return false;
  }

  auto bInitRenderTarget = this->initRenderTarget();
  if (!bInitRenderTarget)
  {
    return false;
  }

  // Set ViewPort
  D3D11_VIEWPORT vp{};
  vp.Width = (FLOAT)sd.BufferDesc.Width;
  vp.Height = (FLOAT)sd.BufferDesc.Height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  m_immediateContext->RSSetViewports(1, &vp);

  return true;
}

bool DeviceResources::initRenderTarget()
{
  // Create a render target view
  auto hr = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)m_renderTarget.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get the back buffer from the swap chain.", L"Error", MB_OK);
    return false;
  }

  hr = m_d3dDevice->CreateRenderTargetView(m_renderTarget.Get(), nullptr, m_renderTargetView.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create render target view.", L"Error", MB_OK);
    return false;
  }
  m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
  return true;
}

bool DeviceResources::resizeRenderTarget(const uint32_t& width, const uint32_t& height, const DXGI_FORMAT& format)
{
  m_swapChainWidth = width;
  m_swapChainHeight = height;
  m_renderTargetView = nullptr;
  m_renderTarget = nullptr;
  auto hr = m_swapchain->ResizeBuffers(0, width, height, format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to resize buffers of swapchain.", L"Error", MB_OK);
    return false;
  }
  auto bInitRenderTarget = this->initRenderTarget();
  if (!bInitRenderTarget)
  {
    return false;
  }
  return true;
}

void DeviceResources::clearRenderTargetView(const float color[4])
{
  m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), color);
}

bool DeviceResources::createTexture(
  ComPtr<ID3D11Texture2D>& resource
  , const uint32_t& width
  , const uint32_t& height
  , const DXGI_FORMAT& format
  , const D3D11_USAGE& heapType
  , const void* data
  , const uint32_t& rowPitch
  , const uint32_t& imageSize
  )
{
  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.MiscFlags = 0;
  desc.Usage = heapType;
  if (heapType == D3D11_USAGE_DYNAMIC)
  {
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  }
  else
  {
    desc.CPUAccessFlags = 0;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
  }

  D3D11_SUBRESOURCE_DATA* ptrInitialData = nullptr;
  D3D11_SUBRESOURCE_DATA initData{0};
  if (data)
  {
    initData.pSysMem = data;
    initData.SysMemPitch = rowPitch;
    initData.SysMemSlicePitch = imageSize;
    ptrInitialData = &initData;
  }

  HRESULT hr = m_d3dDevice->CreateTexture2D(&desc, ptrInitialData, resource.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create ID3D11Texture2D.", L"Error", MB_OK);
    return false;
  }
  return true;
}

bool DeviceResources::createSRV(ID3D11Resource* resource, ComPtr<ID3D11ShaderResourceView>& srv, const DXGI_FORMAT& format, const UINT& mipLevel)
{
  // Create a resource view so we can use the data in a shader
  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
  srv_desc.Format = format;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = mipLevel;
  auto hr = m_d3dDevice->CreateShaderResourceView(resource, &srv_desc, srv.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create the shader resource view.", L"Error", MB_OK);
    return false;
  }
  return true;
}

bool DeviceResources::createUAV(ID3D11Resource* resource, const DXGI_FORMAT& format, ComPtr<ID3D11UnorderedAccessView>& ppUAView)
{
  D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
  uavDesc.Format = format;
  uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
  auto hr = m_d3dDevice->CreateUnorderedAccessView(resource, &uavDesc, ppUAView.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create the unordered access view.", L"Error", MB_OK);
    return false;
  }
  return true;
}

bool DeviceResources::createLinearClampSampler(ComPtr<ID3D11SamplerState>& samplerState)
{
  // Create the sample state
  D3D11_SAMPLER_DESC sampDesc{};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.MipLODBias = 0.0f;
  sampDesc.MaxAnisotropy = 1;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  auto hr = m_d3dDevice->CreateSamplerState(&sampDesc, samplerState.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create SamplerState.", L"Error", MB_OK);
    return false;
  }
  return true;
}

bool DeviceResources::createConstBuffer(void* data, const uint32_t& size, ComPtr<ID3D11Buffer>& ppBuffer)
{
  D3D11_BUFFER_DESC bDesc{};
  bDesc.ByteWidth = size;
  bDesc.Usage = D3D11_USAGE_DYNAMIC;
  bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bDesc.MiscFlags = 0;
  bDesc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA srData{0};
  srData.pSysMem = data;
  auto hr = m_d3dDevice->CreateBuffer(&bDesc, &srData, ppBuffer.GetAddressOf());
  if (hr != S_OK)
  {
    return false;
  }
  return true;
}


bool DeviceResources::updateConstBuffer(void* data, const uint32_t& size, ComPtr<ID3D11Buffer>& ppBuffer)
{
  D3D11_MAPPED_SUBRESOURCE mappedResource{};
  m_immediateContext->Map(ppBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  uint8_t* mappData = (uint8_t*)mappedResource.pData;
  std::memcpy(mappData, data, size);
  m_immediateContext->Unmap(ppBuffer.Get(), 0);
  return true;
}

void DeviceResources::getTextureData(ComPtr<ID3D11Texture2D>& texture, std::vector<uint8_t>& data, uint32_t& width, uint32_t& height, uint32_t& rowPitch)
{
  D3D11_TEXTURE2D_DESC desc{};
  texture->GetDesc(&desc);
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  desc.BindFlags = 0;
  desc.Usage = D3D11_USAGE_STAGING;
  ComPtr<ID3D11Texture2D> stage;
  m_d3dDevice->CreateTexture2D(&desc, nullptr, &stage);
  m_immediateContext->CopyResource(stage.Get(), texture.Get());

  D3D11_MAPPED_SUBRESOURCE mappedResource{};
  auto hr = m_immediateContext->Map(stage.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
  if (hr == S_OK)
  {
    uint8_t* mappData = (uint8_t*)mappedResource.pData;
    width = desc.Width;
    height = desc.Height;
    rowPitch = mappedResource.RowPitch;
    data.resize(mappedResource.DepthPitch);
    memcpy(data.data(), mappData, mappedResource.DepthPitch);
    m_immediateContext->Unmap(stage.Get(), 0);
  }
}
