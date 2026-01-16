#include <memory>
#include <SDL3/SDL.h>
#include <core/renderer/renderer.hpp>
#include <core/window.hpp>
#include <glm/gtc/constants.hpp>
#include "application.hpp"
#include "renderer/render_layer.hpp"

namespace Core {
  Application::Application(const char *name) :
  window(std::make_shared<Window>(name)) {
    Renderer::initialize();
    running = true;
  }

  void Application::run() {
    while (running) {
      pollInputs();

      for (const auto &m_layer: layers)
        m_layer->render();

      window->updateBuffer();

      const double currentTime = static_cast<double>(SDL_GetTicks()) / 1000;
      deltaTime = currentTime - lastFrameTime;
      lastFrameTime = static_cast<double>(currentTime);
    }

    quit();
  }

  bool Application::keyDown(Inputs::KeyboardKey key, Inputs::KeyDetectMode detectMode) const {
    if (!pressedKeys.contains(key)) return false;

    const SDL_KeyboardEvent keyEvent = pressedKeys[key];
    const SDL_Scancode scancode = keyEvent.scancode;
    const SDL_Keycode keycode = SDL_GetKeyFromScancode(keyEvent.scancode, keyEvent.mod, true);

    const bool isScancode = detectMode == Inputs::KeyDetectMode::Keycode && scancode == keyEvent.scancode;
    const bool isKeycode = detectMode == Inputs::KeyDetectMode::Scancode && keycode == keyEvent.key;
    if (isScancode || isKeycode)
      return true;

    return false;
  }

  void Application::pollInputs() {
    SDL_Event windowEvent;
    mouseDelta = glm::zero<glm::vec2>();
    isMouseMoving = false;

    while (SDL_PollEvent(&windowEvent)) {
      const auto pressedKey = static_cast<Inputs::KeyboardKey>(windowEvent.key.scancode);
      switch (windowEvent.type) {
        case SDL_EVENT_QUIT:
          running = false;
          break;

        case SDL_EVENT_KEY_DOWN:
          if (!pressedKeys.contains(pressedKey))
            pressedKeys[pressedKey] = windowEvent.key;
          break;

        case SDL_EVENT_KEY_UP:
          if (pressedKeys.contains(pressedKey))
            pressedKeys.erase(pressedKey);
          break;

        case SDL_EVENT_MOUSE_MOTION:
          isMouseMoving = true;
          mouseDelta = glm::vec2(windowEvent.motion.xrel, windowEvent.motion.yrel);
          break;
      }
    }
  }

  void Application::quit() const {
    SDL_Quit();
  }
}
