#include <iostream>
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <core/logger.hpp>
#include <core/window.hpp>

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  const auto window = Window("Voxel Engine");

  int exit = 0;
  while (!exit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          exit = 1;
          break;
        case SDL_EVENT_KEY_UP:
          if (event.key.key == SDLK_ESCAPE) {
            exit = 1;
          }
          break;
        default:
          break;
      }
    }
    glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    window.UpdateBuffer();
  }

  window.Destroy();
  SDL_Quit();

  return 0;
}
