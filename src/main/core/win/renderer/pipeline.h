
#ifndef PIPELINE_H_
#define PIPELINE_H_

#include "pch.h"

using namespace Microsoft::WRL;

namespace renderer
{

class Pipeline
{
public:
  explicit Pipeline();
  ~Pipeline() = default;

  bool create(D3D11_INPUT_ELEMENT_DESC* input_elements, uint32_t ninput_elements, ComPtr<ID3D11Device> d3dDevice, ComPtr<ID3D11DeviceContext> immediateContext);
  void activate() const;
  void destroy();

private:
  ComPtr<ID3D11VertexShader> m_vs;
  ComPtr<ID3D11InputLayout>  m_inputLayout;
  ComPtr<ID3D11PixelShader>  m_ps;
  ComPtr<ID3D11Device> m_d3dDevice;
  ComPtr<ID3D11DeviceContext> m_immediateContext;

  bool compileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut);
};

} // manager

#endif // PIPELINE_H_
