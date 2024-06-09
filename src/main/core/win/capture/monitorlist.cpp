
#include "monitorlist.h"
#include <map>
#include <algorithm>

using namespace core;

MonitorList::MonitorList(const bool& includeAllMonitors)
{
  m_includeAllMonitors = includeAllMonitors;
  m_monitors = this->enumerateAllMonitors(m_includeAllMonitors);
}

void MonitorList::update()
{
  auto monitors = this->enumerateAllMonitors(m_includeAllMonitors);
  std::map<HMONITOR, MonitorInfo> newMonitors;
  for (auto& monitor : m_monitors)
  {
    newMonitors.insert({ monitor.m_monitorHandle, monitor });
  }
  std::vector<int> monitorIndexesToRemove;
  auto index = 0;
  for (auto& monitor : m_monitors)
  {
    auto search = newMonitors.find(monitor.m_monitorHandle);
    if (search == newMonitors.end())
    {
      monitorIndexesToRemove.push_back(index);
    }
    else
    {
      newMonitors.erase(search);
    }
    index++;
  }

  std::sort(monitorIndexesToRemove.begin(), monitorIndexesToRemove.end(), std::greater<int>());
  for (auto& removalIndex : monitorIndexesToRemove)
  {
    m_monitors.erase(m_monitors.begin() + removalIndex);
  }

  for (auto& pair : newMonitors)
  {
    auto monitor = pair.second;
    m_monitors.push_back(monitor);
  }
}

std::vector<MonitorInfo> MonitorList::enumerateAllMonitors(const bool& includeAllMonitors)
{
  std::vector<MonitorInfo> monitors;
  EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hmon, HDC, LPRECT, LPARAM lparam)
  {
    auto& monitors = *reinterpret_cast<std::vector<MonitorInfo>*>(lparam);
    monitors.push_back(MonitorInfo(hmon));
    return TRUE;
  }, reinterpret_cast<LPARAM>(&monitors));

  if (monitors.size() > 1 && includeAllMonitors)
  {
    auto rc = RECT();
    monitors.push_back(MonitorInfo(nullptr, L"All Displays", rc, rc));
  }
  return monitors;
}
