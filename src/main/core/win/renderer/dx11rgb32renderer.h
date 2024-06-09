
#ifndef DX11_RGB32_RENDERER_H_
#define DX11_RGB32_RENDERER_H_

#include "pch.h"
#include "dx11baserenderer.h"

using namespace Microsoft::WRL;

namespace renderer
{

struct ConstantBufferTextureSize
{
  float scaledTop = 0.0f;
  float scaledLeft = 0.0f;
  float scaledRight = 0.0f;
  float scaledBottom = 0.0f;
  float textureWidth = 0.0f;
  float textureHeight = 0.0f;
  float padding[2]{}; // Padding for 16-byte alignment
};

class DX11RGB32Renderer : public DX11BaseRenderer
{
public:
  explicit DX11RGB32Renderer();
  virtual ~DX11RGB32Renderer();

  bool createScaledAreaCB(const int& scaledTop, const int& scaledLeft, const int& scaledRight, const int& scaledBottom, const int& textureWidth, const int& textureHeight);
  bool updateTexture(const uint8_t* newData, const size_t dataSize) override;

private:
  ComPtr<ID3D11Buffer> m_scaledAreaConsBuffer = nullptr;
};

} // renderer

#endif // DX11_RGB32_RENDERER_H_
