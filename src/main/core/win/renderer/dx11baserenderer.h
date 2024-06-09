
#ifndef DX11_BASE_RENDERER_H_
#define DX11_BASE_RENDERER_H_

#include "pch.h"
#include "pipeline.h"
#include "mesh.h"
#include "deviceresources.h"
#include "nvscaler.h"

using namespace Microsoft::WRL;

namespace renderer
{

class DX11BaseRenderer
{
public:
  explicit DX11BaseRenderer() = default;
  virtual ~DX11BaseRenderer() = default;

  bool init(
    DeviceResources& deviceResources
    , std::shared_ptr<renderer::NVScaler> nvScaler
    , const uint32_t& inputTextureWidth
    , const uint32_t& inputTextureHeight
    , const uint32_t& upscaleTextureWidth
    , const uint32_t& upscaleTextureHeight
    );
  virtual bool updateTexture(const uint8_t* newData, const size_t dataSize) = 0;

protected:
  DeviceResources m_deviceResources;
  Pipeline m_pipeline;
  Mesh m_quad;
  // For input texture
  ComPtr<ID3D11ShaderResourceView> m_inputSRV = nullptr;
  ComPtr<ID3D11Texture2D> m_inputTexture = nullptr;
  ComPtr<ID3D11SamplerState> m_samplerClampLinear = nullptr;
  uint32_t m_inputTextureWidth = 0;
  uint32_t m_inputTextureHeight = 0;
  // For NIS
  ComPtr<ID3D11Texture2D> m_upscaledTexture = nullptr;
  ComPtr<ID3D11UnorderedAccessView> m_upscaledUAV = nullptr;
  ComPtr<ID3D11ShaderResourceView> m_upscaledSRV = nullptr;
  std::shared_ptr<NVScaler> m_nvScaler = nullptr;
  uint32_t m_upscaleTextureWidth = 0;
  uint32_t m_upscaleTextureHeight = 0;

  bool render();

private:
  bool createPipeline();
  bool createMesh();
  bool createInputTexture();
  bool createUpscaledTexture();
  bool createInputSRV();
  bool createUpscaledSRV();
  bool createUAV();
  bool createLinearClampSampler();
};

} // renderer

#endif // DX11_BASE_RENDERER_H_
