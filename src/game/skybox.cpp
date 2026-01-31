#include <core/renderer/renderer.hpp>
#include "camera.hpp"
#include "skybox.hpp"

namespace Game {
  Skybox::Skybox(const Renderer::MeshData &mesh) : cubeMap(Renderer::loadCubeMap(faces)),
  vertexArrayObject(Renderer::createVertexArrayObject()),
  vertexBuffer(Renderer::createVertexBufferObject(mesh.vertexData.data(), mesh.vertexData.size() * sizeof(glm::vec3))),
  indexBuffer(Renderer::createElementBufferObject(mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int))),
  mesh(mesh) {
    Renderer::setVertexData<std::float_t>(0, 3, 3, false, 0, vertexBuffer);
    shader.use();
    shader.updateTexture(cubeMap);
  }

  void Skybox::render(const Camera &camera) const {
    glDepthFunc(GL_LEQUAL);

    shader.use();
    Renderer::useCubeMap(cubeMap);
    shader.setProjectionMatrix(camera.getProjectionMatrix());
    shader.setViewMatrix(glm::mat4(glm::mat3(camera.getViewMatrix())));

    Renderer::drawTriangles(
      vertexArrayObject,
      vertexBuffer,
      indexBuffer,
      mesh.indices.size()
    );
    glDepthFunc(GL_LESS);
  }
}
