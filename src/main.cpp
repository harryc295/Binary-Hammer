#ifdef WIN32
#include <windows.h>
#endif

#include "rendering/render.h"

int main(int, char**)
{
#ifndef _WIN32
  printf("Fatal: Support for other operating systems has not been written yet, please check back soon!\n");
  return 0;
#endif

  render::create();
  while (render::render());
  render::destroy();
  return 0;
}
