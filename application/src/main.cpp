#include <engine/application/graphics_application.hpp>
#include <engine/application/graphics_window.hpp>
#include <engine/events/application_events.hpp>
#include <engine/renderer/renderer_context.hpp>
#include <engine/scene/scene_renderer.hpp>

int main() {
  bool exitRequested = false;
  Engine::Events::EventDispatcher eventDispatcher;
  Engine::Events::EventDispatcher::listen<Engine::Events::ApplicationExitRequest>([&](const Engine::Events::ApplicationExitRequest &event) {
    exitRequested = true;
  });

  const auto rendererContext = Engine::SharedPtr<Engine::Renderer::RendererContext>::create();
  const auto application = Engine::Application::GraphicsApplication(rendererContext);

  const auto sceneRenderer = Engine::Renderer::SceneRenderer(rendererContext);

  const Engine::Application::WindowOptions windowOptions = {
    .title = "Game",
    .width  = 1920,
    .height = 1080,
    .focusWhenFirstShown = true,
    .fullScreen = true,
    .resizable = true,
  };

  auto window = application.addWindow(Engine::SharedPtr<Engine::Application::GraphicsWindow>::create(windowOptions), true);

  while (!exitRequested) {
    eventDispatcher.process();
    application.pollInputs();
    application.render();
  }

  return 0;
}
