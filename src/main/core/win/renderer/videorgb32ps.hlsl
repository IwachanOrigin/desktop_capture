
#include "common.fxh"

Texture2D    txYUV        : register(t0);
SamplerState samLinear    : register(s0);


float4 PS(v2f input) : SV_Target
{
#if 0
  float4 texColor = txYUV.Sample(samLinear, input.Tex);
#else
  float2 textureCoord = float2(
    scaledLeft / textureWidth + input.Tex.x * ((scaledRight - scaledLeft) / textureWidth)
    , scaledTop / textureHeight + input.Tex.y * ((scaledBottom - scaledTop) / textureHeight)
  );
  float4 texColor = txYUV.Sample(samLinear, textureCoord);
#endif
  return texColor;
}
