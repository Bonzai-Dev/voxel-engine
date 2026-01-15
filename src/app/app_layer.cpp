#include <glm/glm.hpp>
#include <core/renderer/renderer.hpp>
#include <util/graphics.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "game/terrain/terrain.hpp"
#include "app_layer.hpp"

AppLayer::AppLayer(const Core::Application &application) :
  RenderLayer(application), camera(application, glm::vec3(0.0f, 20.0f, 0.0f)), terrain(camera) {

}

void AppLayer::render() {
  camera.update();

  Renderer::clearBuffer(Util::Graphics::normalizeColor(glm::vec4(1, 0, 0, 1)));
  terrain.render();
}
