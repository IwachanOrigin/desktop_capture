
#ifndef GPU_HELPER_H_
#define GPU_HELPER_H_

#include "pch.h"

namespace helper
{

enum GPUVenders : UINT
{
    NVIDIA = 0x10DE
  , AMD = 0x1002
  , INTEL = 0x8086
  , UNKNOWN = 0xFFFF
};

class GPUHelper
{
public:
  explicit GPUHelper() = default;
  ~GPUHelper() = default;

  GPUVenders getGPUVender();

}; // GPUHelper

} // helper

#endif // GPU_HELPER_H_

