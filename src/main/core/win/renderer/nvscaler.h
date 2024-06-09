
#ifndef NV_SCALER_H_
#define NV_SCALER_H_

#include "pch.h"
#include "NIS_Config.h"
#include "deviceresources.h"

using namespace Microsoft::WRL;

namespace renderer
{

class NVScaler
{
public:
  explicit NVScaler(DeviceResources& deviceResources);
  ~NVScaler() = default;

  void update(const float& sharpness, const uint32_t& inputWidth, const uint32_t& inputHeight, const uint32_t& outputWidth, const uint32_t& outputHeight);
  void dispatch(ID3D11ShaderResourceView* const* input, ID3D11UnorderedAccessView* const* output);

private:
  DeviceResources& m_deviceResources;
  NISConfig m_config;
  ComPtr<ID3D11ComputeShader> m_computeShader = nullptr;
  ComPtr<ID3D11Buffer> m_csBuffer = nullptr;
  ComPtr<ID3D11Texture2D> m_coefScale = nullptr;
  ComPtr<ID3D11Texture2D> m_coefUsm = nullptr;
  ComPtr<ID3D11ShaderResourceView> m_coefScaleSRV = nullptr;
  ComPtr<ID3D11ShaderResourceView> m_coefUsmSRV = nullptr;
  ComPtr<ID3D11SamplerState> m_linearClampSampler = nullptr;
  uint32_t m_outputWidth = 0;
  uint32_t m_outputHeight = 0;
  uint32_t m_blockWidth = 0;
  uint32_t m_blockHeight = 0;
};

} // renderer

#endif // NV_SCALER_H_

