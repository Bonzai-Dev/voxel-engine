#include <glm/glm.hpp>
#include <util/graphics.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "game/terrain/terrain.hpp"
#include "app_layer.hpp"

AppLayer::AppLayer(const Core::Application &application) :
  RenderLayer(application), terrain(camera), camera(application, glm::vec3(0.0f, 10.0f, 0.0f)) {
}

void AppLayer::render() {
  camera.update();

  Renderer::clearBuffer(Util::Graphics::normalizeColor(glm::vec4(1, 0, 0, 1)));
  terrain.render();
}
