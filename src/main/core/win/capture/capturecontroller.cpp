
#include "capturecontroller.h"
#include <iostream>

core::CaptureController::CaptureController()
{
  // Initialize COM
  winrt::init_apartment(winrt::apartment_type::single_threaded);
}

bool core::CaptureController::init()
{
  // Check to see that capture is supported
  auto isCaptureSupported = winrt::GraphicsCaptureSession::IsSupported();
  if (!isCaptureSupported)
  {
    std::cerr << "Screen capture is not supported on this device for this release of Windows!" << std::endl;
    return false;
  }

  // Create the DispatcherQueue that the compositor needs to run
  m_controller = this->createDispatcherQueueController();
  if (!m_controller)
  {
    std::cerr << "" << std::endl;
    return false;
  }

  // Create the picker
  m_capturePicker = winrt::GraphicsCapturePicker();
  if (!m_capturePicker)
  {
    std::cerr << "" << std::endl;
    return false;
  }

  m_savePicker = winrt::FileSavePicker();
  if (!m_savePicker)
  {
    std::cerr << "" << std::endl;
    return false;
  }

  // Init composition
  m_compositor = winrt::Compositor();
  if (!m_compositor)
  {
    std::cerr << "" << std::endl;
    return false;
  }
  m_root = m_compositor.CreateContainerVisual();
  if (!m_root)
  {
    std::cerr << "" << std::endl;
    return false;
  }
  m_root.RelativeSizeAdjustment({ 1.0f, 1.0f });
  m_root.Size({ -220.0f, 0.0f });
  m_root.Offset({ 220.0f, 0.0f, 0.0f });

  // Create the app
  m_app = std::make_unique<App>(m_root, m_capturePicker, m_savePicker);
  if (!m_app)
  {
    std::cerr << "" << std::endl;
    return false;
  }

  m_isInit = true;
  return true;
}

bool core::CaptureController::setRendererCallback(const core::RendererCallback& callback)
{
  if (!m_isInit)
  {
    return false;
  }

  if (!m_app)
  {
    return false;
  }

  m_app->setRendererCallbackFunc(callback);
  return true;
}

bool core::CaptureController::start(const MonitorInfo& info)
{
  if (!m_isInit)
  {
    return false;
  }

  if (!m_app)
  {
    return false;
  }

  auto item = m_app->tryStartCaptureFromMonitorHandle(info.m_monitorHandle);
  if (!item)
  {
    return false;
  }

  return true;
}

bool core::CaptureController::stop()
{
  if (m_app)
  {
    m_app->stopCapture();
  }
  return true;
}

// Direct3D11CaptureFramePool requires a DispatcherQueue
winrt::Windows::System::DispatcherQueueController core::CaptureController::createDispatcherQueueController()
{
  namespace abi = ABI::Windows::System;

  DispatcherQueueOptions options
  {
      sizeof(DispatcherQueueOptions),
      DQTYPE_THREAD_CURRENT,
      DQTAT_COM_STA
  };

  winrt::Windows::System::DispatcherQueueController controller{ nullptr };
  winrt::check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(winrt::put_abi(controller))));
  return controller;
}


