#include <engine/runtime/events/application_events.hpp>
#include <engine/runtime/renderer/renderer_context.hpp>
#include <engine/runtime/graphics/graphics_application.hpp>
#include <engine/runtime/graphics/graphics_window.hpp>
#include <engine/runtime/game/game_viewport_manager.hpp>

int main() {
  bool exitRequested = false;
  Engine::Events::EventDispatcher eventDispatcher;
  Engine::Events::EventDispatcher::listen<Engine::Events::ApplicationExitRequest>([&](const Engine::Events::ApplicationExitRequest &event) {
    exitRequested = true;
  });

  const auto rendererContext = Engine::SharedPtr<Engine::Renderer::RendererContext>::create();
  const auto application = Engine::Application::GraphicsApplication(rendererContext);

  const auto viewportManager = Engine::Game::GameViewportManager(rendererContext);

  const Engine::Application::WindowOptions windowOptions = {
    .title = "Game",
    .width  = 1920,
    .height = 1080,
    .focusWhenFirstShown = true,
    .fullScreen = true,
    .resizable = true,
  };

  const auto window = application.addWindow(Engine::SharedPtr<Engine::Application::GraphicsWindow>::create(windowOptions), true);

  viewportManager.createViewport(window);

  while (!exitRequested) {
    eventDispatcher.process();
    application.pollInputs();
    viewportManager.render(window);
  }

  return 0;
}
