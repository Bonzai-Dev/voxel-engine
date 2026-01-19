#include <glm/glm.hpp>
#include <util/graphics.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "game/terrain/terrain.hpp"
#include "app_layer.hpp"

AppLayer::AppLayer(const Core::Application &application) :
RenderLayer(application), camera(application, glm::vec3(0, 10.0f, 0)), terrain(camera) {
}

void AppLayer::render() {
  camera.update();

    // const glm::ivec2 cameraPosition = glm::ivec2(camera.getPosition().x, camera.getPosition().z);
  // std::cout << "Camera Position: " << cameraPosition.x << ", " << cameraPosition.y << std::endl;

  Renderer::clearBuffer(Util::Graphics::normalizeColor(glm::vec4(1, 0, 0, 1)));
  terrain.render();
}
