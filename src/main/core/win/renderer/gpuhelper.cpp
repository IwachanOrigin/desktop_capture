
#include "gpuhelper.h"

using namespace helper;
using namespace Microsoft::WRL;

GPUVenders GPUHelper::getGPUVender()
{
  ComPtr<IDXGIFactory> pFactory = nullptr;
  auto hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)pFactory.GetAddressOf());
  if (FAILED(hr))
  {
    std::cerr << "Failed to create DXGI Factory." << std::endl;
    return GPUVenders::UNKNOWN;
  }

  ComPtr<IDXGIAdapter> pAdapter = nullptr;
  DXGI_ADAPTER_DESC desc{};
  GPUVenders venderID = GPUVenders::UNKNOWN;
  for (UINT i = 0; pFactory->EnumAdapters(i, pAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; i++)
  {
    pAdapter->GetDesc(&desc);
    if (desc.VendorId)
    {
      venderID = (GPUVenders)desc.VendorId;
      break; // Get only first adapter that has a VendorID.
    }
  }
  return venderID;
}
