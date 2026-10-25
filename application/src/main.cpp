#include <engine/application/window.hpp>
#include "application.hpp"
#include "app_layer.hpp"

int main() {
  const Application::Application application("Vulkan Template");
  // application.createWindow({
  //   .mouseLocked = false,
  //   .fullScreen = false,
  //   .vsync = true,
  //   .resizable = true,
  //   .width = 1920,
  //   .height = 1080,
  //   .windowName = "Game"
  // });

  // application.addLayer<Application::AppLayer>();
  application.run();

  return 0;
}
