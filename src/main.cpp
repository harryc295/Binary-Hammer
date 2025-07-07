#ifdef WIN32
#include <windows.h>
#endif

#include "rendering/render.h"

int main(int, char**)
{
#ifdef WIN32
  HWND console = GetConsoleWindow();
  FreeConsole();
  CloseWindow(console);
#endif

  if (!render::create())
    return 1;
  while (render::render())
  {
    // while rendering
  }
  render::destroy();

  return 0;
}
