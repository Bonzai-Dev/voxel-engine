#include <engine/graphics/graphical_application.hpp>
#include <engine/graphics/graphical_window.hpp>
#include <engine/events/application_events.hpp>

int main() {
  bool exitRequested = false;
  Engine::Events::EventDispatcher eventDispatcher;
  Engine::Events::EventDispatcher::listen<Engine::Events::ApplicationExitRequest>([&](const Engine::Events::ApplicationExitRequest &event) {
    exitRequested = true;
  });

  Engine::Graphics::GraphicalApplication application;

  Engine::Graphics::WindowOptions windowOptions = {};
  windowOptions.width  = 1920;
  windowOptions.height = 1080;
  windowOptions.title = "Game";
  windowOptions.focusWhenFirstShown = true;
  windowOptions.fullScreen = true;
  windowOptions.resizable = true;

  const auto window = Engine::SharedPtr<Engine::Graphics::GraphicalWindow>().create(windowOptions);
  application.addWindow(window, true);

  while (!exitRequested) {
    eventDispatcher.process();
    application.pollInputs();
  }

  return 0;
}
