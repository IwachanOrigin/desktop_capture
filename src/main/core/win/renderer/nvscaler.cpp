
#include "nvscaler.h"
#include "Utilities.h"
#include "DXUtilities.h"
#include "gpuhelper.h"
#include <filesystem>

using namespace renderer;

NVScaler::NVScaler(DeviceResources& deviceResources)
  : m_deviceResources(deviceResources)
  , m_outputWidth(1)
  , m_outputHeight(1)
{
  helper::GPUHelper gpuhelper;
  auto gpuVender = gpuhelper.getGPUVender();
  NISGPUArchitecture nisGPUArch = NISGPUArchitecture::NVIDIA_Generic;
  switch(gpuVender)
  {
    case helper::GPUVenders::INTEL:
    {
      nisGPUArch = NISGPUArchitecture::Intel_Generic;
    }
    break;

    case helper::GPUVenders::AMD:
    {
      nisGPUArch = NISGPUArchitecture::AMD_Generic;
    }
    break;

    case helper::GPUVenders::NVIDIA:
    default:
    {
      nisGPUArch = NISGPUArchitecture::NVIDIA_Generic;
    }
    break;
  }

  NISOptimizer opt(true, nisGPUArch);
  m_blockWidth = opt.GetOptimalBlockWidth();
  m_blockHeight = opt.GetOptimalBlockHeight();
  auto threadGroupSize = opt.GetOptimalThreadGroupSize();

  DX::Defines defines;
  defines.add("NIS_SCALER", true);
  defines.add("NIS_HDR_MODE", uint32_t(NISHDRMode::None));
  defines.add("NIS_BLOCK_WIDTH", m_blockWidth);
  defines.add("NIS_BLOCK_HEIGHT", m_blockHeight);
  defines.add("NIS_THREAD_GROUP_SIZE", threadGroupSize);

  std::string shaderPath = "Shader";
  std::string shaderName = "NIS_Main.hlsl";
  std::string shaderFolder;
  if (std::filesystem::exists(shaderPath + "/" + shaderName))
  {
    shaderFolder = shaderPath;
  }
  if (shaderFolder.empty())
  {
    throw std::runtime_error("Shader file not found" + shaderName);
  }

  std::wstring wShaderFilename = widen(shaderFolder + "/" + "NIS_Main.hlsl");
  DX::IncludeHeader includeHeader({ shaderFolder });
  DX::CompileComputeShader(m_deviceResources.device().Get(),
                           wShaderFilename.c_str(),
                           "main",
                           &m_computeShader,
                           defines.get(),
                           &includeHeader);

  const int rowPitch = kFilterSize * 4;
  const int imageSize = rowPitch * kPhaseCount;

  m_deviceResources.createTexture(
    m_coefScale
    , kFilterSize / 4
    , kPhaseCount
    , DXGI_FORMAT_R32G32B32A32_FLOAT
    , D3D11_USAGE_DEFAULT
    , coef_scale
    , rowPitch
    , imageSize);
  m_deviceResources.createTexture(
    m_coefUsm
    , kFilterSize / 4
    , kPhaseCount
    , DXGI_FORMAT_R32G32B32A32_FLOAT
    , D3D11_USAGE_DEFAULT
    , coef_usm
    , rowPitch
    , imageSize);
  m_deviceResources.createSRV(m_coefScale.Get(), m_coefScaleSRV, DXGI_FORMAT_R32G32B32A32_FLOAT);
  m_deviceResources.createSRV(m_coefUsm.Get(), m_coefUsmSRV, DXGI_FORMAT_R32G32B32A32_FLOAT);
  m_deviceResources.createLinearClampSampler(m_linearClampSampler);
  m_deviceResources.createConstBuffer(&m_config, sizeof(NISConfig), m_csBuffer);
}

void NVScaler::update(const float& sharpness, const uint32_t& inputWidth, const uint32_t& inputHeight, const uint32_t& outputWidth, const uint32_t& outputHeight)
{
  NVScalerUpdateConfig(m_config, sharpness,
                       0, 0, inputWidth, inputHeight, inputWidth, inputHeight,
                       0, 0, outputWidth, outputHeight, outputWidth, outputHeight,
                       NISHDRMode::None);

  m_deviceResources.updateConstBuffer(&m_config, sizeof(NISConfig), m_csBuffer);
  m_outputWidth = outputWidth;
  m_outputHeight = outputHeight;
}

void NVScaler::dispatch(ID3D11ShaderResourceView* const* input, ID3D11UnorderedAccessView* const* output)
{
  auto context = m_deviceResources.context();
  context->CSSetShaderResources(0, 1, input);
  context->CSSetUnorderedAccessViews(0, 1, output, nullptr);
  context->CSSetShaderResources(1, 1, m_coefScaleSRV.GetAddressOf());
  context->CSSetShaderResources(2, 1, m_coefUsmSRV.GetAddressOf());
  context->CSSetSamplers(0, 1, m_linearClampSampler.GetAddressOf());
  context->CSSetConstantBuffers(0, 1, m_csBuffer.GetAddressOf());
  context->CSSetShader(m_computeShader.Get(), nullptr, 0);
  context->Dispatch(UINT(std::ceil(m_outputWidth / float(m_blockWidth))), UINT(std::ceil(m_outputHeight / float(m_blockHeight))), 1);
}

