
#ifndef WIN32_MESSAGE_HANDLER_H_
#define WIN32_MESSAGE_HANDLER_H_

#include "pch.h"

namespace message_handler
{

class Win32MessageHandler
{
public:
  static Win32MessageHandler &getInstance();

  bool init(HINSTANCE hinst, int nCmdShow, const uint32_t& windowWidth, const uint32_t& windowHeight);
  int run();
  HWND hwnd() { return m_hwnd; }

protected:
  static LRESULT CALLBACK MessageProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
  Win32MessageHandler();
  ~Win32MessageHandler();
  explicit Win32MessageHandler(const Win32MessageHandler &);
  Win32MessageHandler &operator=(const Win32MessageHandler &);

  HWND m_hwnd;
};

} // message_handler

#endif // WIN32_MESSAGE_HANDLER_H_
