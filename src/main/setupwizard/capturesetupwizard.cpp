
#include "capturesetupwizard.h"
#include "monitorlist.h"
#include "areaselecter.h"
#include <iostream>

namespace sw = setup_wizard;

std::tuple<int, int, int, int, int, int, int, int, int> sw::CaptureSetupWizard::setup(const std::vector<core::MonitorInfo>& monitors)
{
  // Select the display number.
  std::string input = "";
  std::wcout << "Please select the capture desktop number." << std::endl;
  for(int i = 0; i < monitors.size(); i++)
  {
    auto monitor = monitors[i];
    std::wcout << i << " : " << monitor.m_displayName << std::endl;
  }
  std::wcout << std::endl;

  int captureIndex = 0;
  do
  {
    std::wcout << ">> ";
    std::cin >> input;
    try
    {
      captureIndex = std::stoi(input);
    }
    catch(const std::invalid_argument& e)
    {
      std::cerr << e.what() << std::endl;
    }
    catch (const std::out_of_range& e)
    {
      std::cerr << e.what() << std::endl;
    }
  } while (captureIndex >= monitors.size() || captureIndex < 0);

  // Get monitor and monitor width, height
  auto& monitor = monitors.at(captureIndex);
  auto rcMonitor = monitor.m_rcMonitor;
  auto monitorWidth = std::abs(rcMonitor.right - rcMonitor.left);
  auto monitorHeight = std::abs(rcMonitor.bottom - rcMonitor.top);

  // Select the mode of adopting the rectangle in desktop.
  input.clear();
  std::wcout << "Please select the mode of adopting the rectangle in desktop." << std::endl;
  std::wcout << "0 : None" << std::endl;
  std::wcout << "1 : Select the rectangle area." << std::endl << std::endl;
  do
  {
    std::wcout << ">> ";
    std::cin >> input;
    try
    {
      int m = std::stoi(input);
      m_adoptRectArea = static_cast<sw::AdoptRectArea>(m);
    }
    catch(const std::invalid_argument& e)
    {
      std::cerr << e.what() << std::endl;
    }
    catch (const std::out_of_range& e)
    {
      std::cerr << e.what() << std::endl;
    }
  }
  while(m_adoptRectArea < sw::AdoptRectArea::NONE || m_adoptRectArea >= sw::AdoptRectArea::UNKNOWN);

  int areaSelectX0 = 0;
  int areaSelectY0 = 0;
  int areaSelectX1 = monitorWidth;
  int areaSelectY1 = monitorHeight;
  bool areaSelected = false;
  if (m_adoptRectArea == sw::AdoptRectArea::RECTANGLE)
  {
    common::AreaSelector areaSelector;
    if (areaSelector.init())
    {
      auto cursorCenterX = (rcMonitor.left + rcMonitor.right) / 2;
      auto cursorCenterY = (rcMonitor.top + rcMonitor.bottom) / 2;

      std::cout << "cursorPosX  : " << cursorCenterX << std::endl;
      std::cout << "cursorPosY : " << cursorCenterY << std::endl;
      // Move mouse cursor to the center of capture monitor 
      SetCursorPos(cursorCenterX, cursorCenterY);

      // Start to select the rectangle area
      auto [x0, y0, x1, y1] = areaSelector.run();
      areaSelectX0 = x0;
      areaSelectY0 = y0;
      areaSelectX1 = x1;
      areaSelectY1 = y1;
      areaSelected = true;
      std::cout << "x0: " << areaSelectX0 << std::endl;
      std::cout << "y0: " << areaSelectY0 << std::endl;
      std::cout << "x1: " << areaSelectX1 << std::endl;
      std::cout << "y1: " << areaSelectY1 << std::endl;
    }
  }

  // Select the mode of desktop capture
  input.clear();
  std::wcout << "Please select the capture mode." << std::endl;
  std::wcout << "0 : Default" << std::endl;
  std::wcout << "1 : 4K upscale" << std::endl << std::endl;
  do
  {
    std::wcout << ">> ";
    std::cin >> input;
    try
    {
      int m = std::stoi(input);
      m_capMode = static_cast<sw::CaptureMode>(m);
    }
    catch(const std::invalid_argument& e)
    {
      std::cerr << e.what() << std::endl;
    }
    catch (const std::out_of_range& e)
    {
      std::cerr << e.what() << std::endl;
    }
  } while (m_capMode < sw::CaptureMode::DEFAULT || m_capMode >= sw::CaptureMode::UNKNOWN);

  // Setup the output window size
  switch(m_capMode)
  {
    case setup_wizard::CaptureMode::UPSCALE_4K:
    {
      m_windowWidth = 3840;
      m_windowHeight = 2160;
      float scaleX = static_cast<float>((float)m_windowWidth / monitorWidth);
      float scaleY = static_cast<float>((float)m_windowHeight / monitorHeight);
      areaSelectX0 *= scaleX;
      areaSelectY0 *= scaleY;
      areaSelectX1 *= scaleX;
      areaSelectY1 *= scaleY;
    }
    break;

    case setup_wizard::CaptureMode::DEFAULT:
    default:
    {
      m_windowWidth = monitorWidth;
      m_windowHeight = monitorHeight;
    }
    break;
  }

  return std::make_tuple(captureIndex, monitorWidth, monitorHeight, m_windowWidth, m_windowHeight, areaSelectX0, areaSelectY0, areaSelectX1, areaSelectY1);
}

