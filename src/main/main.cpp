
#if _WIN32
#include "pch.h"
#endif

#include <iostream>
#include "win32messagehandler.h"
#include "dx11rgb32renderer.h"
#include "monitorlist.h"
#include "capturecontroller.h"
#include "deviceresources.h"
#include "nvscaler.h"
#include "capturesetupwizard.h"

namespace mh = message_handler;

int main(int argc, char *argv[])
{
  // Create monitorlist
  core::MonitorList monitorList;
  // Get monitor
  auto monitors = monitorList.currentMonitors();

  // Create setupwizard
  auto csw = setup_wizard::CaptureSetupWizard();
  auto [desktopIndex, capMonWidth, capMonHeight, upscaleWidth, upscaleHeight, areaSelectX0, areaSelectY0, areaSelectX1, areaSelectY1] = csw.setup(monitors);

  // Create main window
  auto result = mh::Win32MessageHandler::getInstance().init((HINSTANCE)0, 1, upscaleWidth, upscaleHeight);
  if (!result)
  {
    std::cerr << "Failed to create main window." << std::endl;
    return -1;
  }

  // Get the render target hwnd.
  auto previewHwnd = mh::Win32MessageHandler::getInstance().hwnd();
  if (!previewHwnd)
  {
    std::cerr << "Failed to get the render target hwnd." << std::endl;
    return -1;
  }

  renderer::DeviceResources deviceResources;
  // Create device resources.
  deviceResources.create(previewHwnd, upscaleWidth, upscaleHeight, 60);

  renderer::DX11RGB32Renderer renderer;
  switch(csw.captureMode())
  {
    case setup_wizard::CaptureMode::UPSCALE_4K:
    {
      // Create NvScaler
      std::shared_ptr<renderer::NVScaler> nvScaler = std::make_shared<renderer::NVScaler>(deviceResources);
      nvScaler->update(100.0f, capMonWidth, capMonHeight, upscaleWidth, upscaleHeight);

      // Create renderer.
      renderer.init(deviceResources, nvScaler, capMonWidth, capMonHeight, upscaleWidth, upscaleHeight);
      renderer.createScaledAreaCB(areaSelectY0, areaSelectX0, areaSelectX1, areaSelectY1, upscaleWidth, upscaleHeight);
    }
    break;

    case setup_wizard::CaptureMode::DEFAULT:
    default:
    {
      // Create renderer.
      renderer.init(deviceResources, nullptr, capMonWidth, capMonHeight, upscaleWidth, upscaleHeight);
      renderer.createScaledAreaCB(areaSelectY0, areaSelectX0, areaSelectX1, areaSelectY1, upscaleWidth, upscaleHeight);
    }
    break;
  }

  // Create capture class.
  core::CaptureController captureController;
  result = captureController.init();
  if (!result)
  {
    std::cerr << "Failed to init capture controller." << std::endl;
    return -1;
  }

  // Set callback function.
  captureController.setRendererCallback(std::bind(&renderer::DX11RGB32Renderer::updateTexture, renderer, std::placeholders::_1, std::placeholders::_2));
  // Start capture.
  captureController.start(monitors.at(desktopIndex));

  // Start message loop
  mh::Win32MessageHandler::getInstance().run();

  // Stop capture.
  captureController.stop();

  return 0;
}

