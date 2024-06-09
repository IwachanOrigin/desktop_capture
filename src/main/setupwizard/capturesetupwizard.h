
#ifndef CAPTURE_SETUP_WIZARD_H_
#define CAPTURE_SETUP_WIZARD_H_

#include <tuple>
#include <vector>
#include "monitorlist.h"

namespace setup_wizard
{

enum class AdoptRectArea : int
{
  NONE = 0
  , RECTANGLE
  , UNKNOWN    // This parameter is checking parameter.
};

enum class UpscaleMode : int
{
  DEFAULT = 0
  , UPSCALE_4K
  , UNKNOWN    // This parameter is checking parameter.
};

class CaptureSetupWizard
{
public:
  explicit CaptureSetupWizard() = default;
  ~CaptureSetupWizard() = default;

  std::tuple<int, int, int, int, int, int, int, int, int> setup(const std::vector<core::MonitorInfo>& monitors);

  UpscaleMode captureMode() const { return m_capMode; }
  int captureIndex() const { return m_captureIndex; }

private:
  AdoptRectArea m_adoptRectArea = AdoptRectArea::NONE;
  UpscaleMode m_capMode = UpscaleMode::DEFAULT;
  int m_captureIndex = -1;
  int m_windowWidth = 0;
  int m_windowHeight = 0;
};

} // setup_wizard

#endif // CAPTURE_SETUP_WIZARD_H_
