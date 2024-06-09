
#ifndef MONITOR_LIST_H_
#define MONITOR_LIST_H_

#include "pch.h"
#include <string>
#include <vector>

namespace core
{

struct MonitorInfo
{
  MonitorInfo(HMONITOR monitorHandle)
  {
    m_monitorHandle = monitorHandle;
    MONITORINFOEX monitorInfo = { sizeof(monitorInfo)};
    winrt::check_bool(GetMonitorInfo(monitorHandle, &monitorInfo));
    std::wstring displayName(monitorInfo.szDevice);
    m_displayName = displayName;
    m_rcMonitor = monitorInfo.rcMonitor;
    m_rcWork = monitorInfo.rcWork;
  }
  MonitorInfo(HMONITOR monitorHandle, const std::wstring& displayName, const RECT& rcMonitor, const RECT& rcWork)
  {
    m_monitorHandle = monitorHandle;
    m_displayName = displayName;
    m_rcMonitor = rcMonitor;
    m_rcWork = rcWork;
  }
  HMONITOR m_monitorHandle;
  std::wstring m_displayName;
  RECT m_rcMonitor;
  RECT m_rcWork;

  bool operator==(const MonitorInfo& monitor) { return m_monitorHandle == monitor.m_monitorHandle; }
  bool operator!=(const MonitorInfo& monitor) { return !(*this == monitor); }
};

class MonitorList
{
public:
  MonitorList(const bool& includeAllMonitors = false);

  void update();
  std::vector<MonitorInfo> currentMonitors() const { return m_monitors; }

private:
  std::vector<MonitorInfo> enumerateAllMonitors(const bool& includeAllMonitors);

  std::vector<MonitorInfo> m_monitors;
  bool m_includeAllMonitors = false;
};

} // core

#endif // MONITOR_LIST_H_
