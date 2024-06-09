
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
#include "nvsharpen.h"
#include "capturesetupwizard.h"

namespace mh = message_handler;

constexpr LONG MAX_UPSCALE_WIDTH = 3840;
constexpr LONG MAX_UPSCALE_HEIGHT = 2160;

int main(int argc, char *argv[])
{
  // Create monitorlist
  core::MonitorList monitorList;
  // Get monitor
  auto monitors = monitorList.currentMonitors();

  // Create setupwizard
  auto csw = setup_wizard::CaptureSetupWizard();
  auto [desktopIndex, capMonWidth, capMonHeight, upscaleWidth, upscaleHeight, areaSelectX0, areaSelectY0, areaSelectX1, areaSelectY1] = csw.setup(monitors);

  // Setup the output window size
  int outWinWidth = 0;
  int outWinHeight = 0;
  bool is4K = false;
  bool isDestWinSmallerThanCapWin = false;
  switch(csw.captureMode())
  {
    case setup_wizard::UpscaleMode::UPSCALE_4K:
    {
      outWinWidth = (upscaleWidth < MAX_UPSCALE_WIDTH) ? upscaleWidth : MAX_UPSCALE_WIDTH;
      outWinHeight = (upscaleHeight < MAX_UPSCALE_HEIGHT) ? upscaleHeight : MAX_UPSCALE_HEIGHT;
      is4K = true;
      if (outWinWidth < capMonWidth || outWinHeight < capMonHeight)
      {
        // Is the destination window smaller than the capture window?
        isDestWinSmallerThanCapWin = true;
      }
    }
    break;

    case setup_wizard::UpscaleMode::DEFAULT:
    default:
    {
      outWinWidth = upscaleWidth;
      outWinHeight = upscaleHeight;
    }
    break;
  }

  // Calculate scale.
  float scaleX = static_cast<float>((float)outWinWidth / capMonWidth);
  float scaleY = static_cast<float>((float)outWinHeight / capMonHeight);
  areaSelectX0 *= scaleX;
  areaSelectY0 *= scaleY;
  areaSelectX1 *= scaleX;
  areaSelectY1 *= scaleY;

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

  // Create device resources.
  renderer::DeviceResources deviceResources;
  renderer::DX11RGB32Renderer renderer;
  if (is4K)
  {
    if (isDestWinSmallerThanCapWin)
    {
      // Create NvSharpen
      auto nvShapen = std::make_shared<renderer::NVSharpen>(deviceResources);
      nvShapen->update(100.0f, capMonWidth, capMonHeight);
      // Create renderer.
      renderer.init(deviceResources, nullptr, nvShapen, capMonWidth, capMonHeight, capMonWidth, capMonHeight);
      renderer.createScaledAreaCB(areaSelectY0, areaSelectX0, areaSelectX1, areaSelectY1, outWinWidth, outWinHeight);
    }
    else
    {
      // Create NvScaler
      auto nvScaler = std::make_shared<renderer::NVScaler>(deviceResources);
      nvScaler->update(100.0f, capMonWidth, capMonHeight, outWinWidth, outWinHeight);
      // Create renderer.
      renderer.init(deviceResources, nvScaler, nullptr, capMonWidth, capMonHeight, outWinWidth, outWinHeight);
      renderer.createScaledAreaCB(areaSelectY0, areaSelectX0, areaSelectX1, areaSelectY1, outWinWidth, outWinHeight);
    }
  }
  else
  {
    // Create renderer.
    renderer.init(deviceResources, nullptr, nullptr, capMonWidth, capMonHeight, outWinWidth, outWinHeight);
    renderer.createScaledAreaCB(areaSelectY0, areaSelectX0, areaSelectX1, areaSelectY1, outWinWidth, outWinHeight);
  }
  deviceResources.create(previewHwnd, upscaleWidth, upscaleHeight, 60);

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

