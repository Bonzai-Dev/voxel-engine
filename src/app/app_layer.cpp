#include <glm/glm.hpp>
#include <util/graphics.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "terrain/world.hpp"
#include "app_layer.hpp"

AppLayer::AppLayer(const Core::Application &application) :
RenderLayer(application), camera(application, glm::vec3(0, 100.0f, 0)),
world(camera) {
}

void AppLayer::render() {
  camera.update();

  if (application.keyDown(Core::Inputs::KeyboardKey::KeyP, Core::Inputs::Keycode))
    wireframe = true;
  if (application.keyDown(Core::Inputs::KeyboardKey::KeyO, Core::Inputs::Keycode))
    wireframe = false;

  if (wireframe)
    Renderer::setFillMode(Renderer::MeshFillMode::Wireframe);
  if (!wireframe)
    Renderer::setFillMode(Renderer::MeshFillMode::Solid);

  Renderer::clearBuffer(Util::Graphics::normalizeColor(glm::vec4(87, 178, 255, 1)));
  world.render();
}
