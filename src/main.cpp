#include <iostream>
#include <SDL3/SDL.h>
#include <core/logger.hpp>
#include <core/window.hpp>

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  const auto window = Window("Voxel Engine");

  SDL_Quit();

  return 0;
}
