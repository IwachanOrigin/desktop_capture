
#ifndef CAPTURE_CONTROLLER_H_
#define CAPTURE_CONTROLLER_H_

#include "pch.h"
#include "app.h"
#include "monitorlist.h"
#include "renderercallback.h"

#include <ShObjIdl.h>
#include <memory>

namespace winrt
{
using namespace Windows::Foundation::Metadata;
using namespace Windows::Graphics::Capture;
using namespace Windows::Storage::Pickers;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;
}

namespace core
{

class CaptureController
{
public:
  explicit CaptureController();
  ~CaptureController() = default;

  bool init();
  bool setRendererCallback(const RendererCallback& callback);
  bool start(const MonitorInfo& info);
  bool stop();

private:
  winrt::Windows::System::DispatcherQueueController createDispatcherQueueController();

  std::unique_ptr<App> m_app = nullptr;
  winrt::Windows::System::DispatcherQueueController m_controller{nullptr};
  winrt::GraphicsCapturePicker m_capturePicker{nullptr};
  winrt::FileSavePicker m_savePicker{nullptr};
  winrt::Compositor m_compositor{nullptr};
  winrt::ContainerVisual m_root{nullptr};
  std::atomic_bool m_isInit = false;
};

} // core

#endif // CAPTURE_CONTROLLER_H_


