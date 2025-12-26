#include <memory>
#include <SDL3/SDL.h>
#include <core/renderer/renderer.hpp>
#include <core/window.hpp>
#include <glm/gtc/constants.hpp>
#include "application.hpp"
#include "render_layer.hpp"

namespace Core {
  Application::Application(const char *name) :
  window(std::make_shared<Window>(name)) {
    Renderer::initialize();
    m_running = true;
  }

  void Application::Run() {
    while (m_running) {
      PollInputs();

      for (const auto &m_layer: m_layers)
        m_layer->Render();

      window->UpdateBuffer();

      const double currentTime = static_cast<double>(SDL_GetTicks()) / 1000;
      deltaTime = currentTime - m_lastFrameTime;
      m_lastFrameTime = static_cast<double>(currentTime);
    }

    Quit();
  }

  bool Application::KeyDown(Inputs::KeyboardKey key, Inputs::KeyDetectMode detectMode) const {
    if (!m_pressedKeys.contains(key)) return false;

    const SDL_KeyboardEvent keyEvent = m_pressedKeys[key];
    const SDL_Scancode scancode = keyEvent.scancode;
    const SDL_Keycode keycode = SDL_GetKeyFromScancode(keyEvent.scancode, keyEvent.mod, true);

    const bool isScancode = detectMode == Inputs::KeyDetectKeycode && scancode == keyEvent.scancode;
    const bool isKeycode = detectMode == Inputs::KeyDetectScancode && keycode == keyEvent.key;
    if (isScancode || isKeycode)
      return true;

    return false;
  }

  void Application::PollInputs() {
    SDL_Event windowEvent;
    mouseDelta = glm::zero<glm::vec2>();
    mouseMoving = false;

    while (SDL_PollEvent(&windowEvent)) {
      const auto pressedKey = static_cast<Inputs::KeyboardKey>(windowEvent.key.scancode);
      switch (windowEvent.type) {
        case SDL_EVENT_QUIT:
          m_running = false;
          break;

        case SDL_EVENT_KEY_DOWN:
          if (!m_pressedKeys.contains(pressedKey))
            m_pressedKeys[pressedKey] = windowEvent.key;
          break;

        case SDL_EVENT_KEY_UP:
          if (m_pressedKeys.contains(pressedKey))
            m_pressedKeys.erase(pressedKey);
          break;

        case SDL_EVENT_MOUSE_MOTION:
          mouseMoving = true;
          mouseDelta = glm::vec2(windowEvent.motion.xrel, windowEvent.motion.yrel);
          break;
      }
    }
  }

  void Application::Quit() const {
    SDL_Quit();
  }
}
