#include <glm/glm.hpp>
#include <util/graphics.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "game/terrain/world.hpp"
#include "app_layer.hpp"

AppLayer::AppLayer(const Core::Application &application) :
RenderLayer(application), camera(application, glm::vec3(15, 100.0f, 15)), terrain(camera) {
}

void AppLayer::render() {
  camera.update();

  Renderer::clearBuffer(Util::Graphics::normalizeColor(glm::vec4(87, 178, 255, 1)));
  terrain.render();
}
