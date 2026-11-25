#include <events/application_events.hpp>
#include <renderer/renderer_context.hpp>
#include <graphics/graphics_application.hpp>
#include <graphics/graphics_window.hpp>
#include <game/game_viewport_manager.hpp>

int main() {
  bool exitRequested = false;
  Engine::Events::EventDispatcher eventDispatcher;
  Engine::Events::EventDispatcher::listen<Engine::Events::ApplicationExitRequest>([&](const Engine::Events::ApplicationExitRequest &event) {
    exitRequested = true;
  });

  const auto rendererContext = Engine::SharedPtr<Engine::Renderer::RendererContext>::create();

  // GraphicsWindows SHOULD OWN a root widget which owns other widgets
  //


  // Owns viewports, needs to have a create viewport function of somesort that returns a handle to be passed into viewport manager. a viewport would basically be where we draw our scene.
  // Look into GetViewportRHI function from unreal (slaterhirenderer.cpp)
  const auto application = Engine::Application::GraphicsApplication(rendererContext); // Manages IO stuff for graphics apps no rendering

  // Manages all ui stuff?
  const auto viewportManager = Engine::Game::GameViewportManager(rendererContext);

  Engine::Application::WindowOptions windowOptions = {};
  windowOptions.title = "Game";
  windowOptions.width  = 1920;
  windowOptions.height = 1080;
  windowOptions.focusWhenFirstShown = true;
  windowOptions.fullScreen = true;
  windowOptions.resizable = true;

  const auto window = application.addWindow(Engine::SharedPtr<Engine::Application::GraphicsWindow>::create(windowOptions), true);

  viewportManager.createViewport(window);

  while (!exitRequested) {
    eventDispatcher.process();
    application.pollInputs();
    viewportManager.render(window);
  }

  return 0;
}
