#include <core/application.hpp>
#include "app_layer.hpp"

int main() {
  const Core::Application application("Vulkan Template");
  application.createWindow({
    .mouseLocked = false,
    .fullScreen = false,
    .vsync = true,
    .resizable = true,
    .width = 1920,
    .height = 1080,
    .windowName = "Game"
  });

  application.addLayer<AppLayer>();
  application.run();

  return 0;
}
