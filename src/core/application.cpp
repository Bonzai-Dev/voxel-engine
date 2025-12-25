#include <memory>
#include <SDL3/SDL.h>
#include <core/renderer/renderer.hpp>
#include <core/window.hpp>
#include "application.hpp"

namespace Core {
  Application::Application(const char *name) {
    SDL_Init(SDL_INIT_VIDEO);
    m_name = name;
    m_window = std::make_unique<Window>(name);
    Renderer::initialize();
    m_running = true;
  }

  void Application::Run() {
    while (m_running) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_EVENT_QUIT:
            m_running = false;
            break;
        }
      }

      for (const auto & m_layer : m_layers)
        m_layer->Render();

      m_window->UpdateBuffer();

      const double currentTime = static_cast<double>(SDL_GetTicks()) / 1000;
      deltaTime = currentTime - m_lastFrameTime;
      m_lastFrameTime = static_cast<double>(currentTime);
    }

    Quit();
  }

  void Application::Quit() const {
    SDL_Quit();
  }
}
