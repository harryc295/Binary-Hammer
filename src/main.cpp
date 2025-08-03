#ifdef _WIN32
#include <windows.h>
#endif

#include "rendering/ui.h"
#include "logger.h"
#include "config.h"

#include <string>
#include <iostream>

int main(int, char**)
{
#ifndef _WIN32
  Logger::get()->log("Fatal Error: Support for other operating systems has not been written yet, please check back soon!\n", "Main");
  return 0;
#endif

  HWND console = GetConsoleWindow();
  FreeConsole();
  CloseWindow(console);

  UI* window_instance = UI::get();
  Config* config_instance = Config::get();

  if (!window_instance->create_window()) {
    Logger::get()->log("Error: Unable to create a window.", "Main");
    return 1;
  }
  
  while (window_instance->render_frame());
  
  window_instance->destroy_window();

  return 0;
}
