#include <core/renderer/renderer.hpp>
#include "skybox.hpp"

static inline std::array vertices = {
  // positions
  -1.0f,  1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

  -1.0f,  1.0f, -1.0f,
   1.0f,  1.0f, -1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f,  1.0f
};

namespace Game {
  // texture(Renderer::loadCubeMap(faces)),
  // vertexArrayObject(Renderer::createVertexArrayObject()),
  // vertexBuffer(Renderer::createVertexBufferObject(mesh.vertexData.data(), mesh.vertexData.size() * sizeof(float))),
  // indexBuffer(Renderer::createElementBufferObject(mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int))),
  //
  Skybox::Skybox(const Renderer::MeshData &mesh) : mesh(mesh) {
    texture = Renderer::loadCubeMap(faces);
    vertexArrayObject = Renderer::createVertexArrayObject();
    vertexBuffer = Renderer::createVertexBufferObject(vertices.data(), vertices.size() * sizeof(float));
    // indexBuffer = Renderer::createElementBufferObject(mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int));

    // Renderer::setVertexData<std::float_t>(0, 3, 5, false, 0, vertexBuffer);
    // Renderer::setVertexData<std::float_t>(1, 2, 5, false, 3, vertexBuffer);

    shader.use();
    shader.updateTexture(texture);
  }

  void Skybox::render() const {
   Renderer::useVertexArrayObject(vertexArrayObject);
   // glDepthMask(GL_FALSE);
   //  Renderer::drawTriangles(
   //    vertexArrayObject,
   //    vertexBuffer,
   //    indexBuffer,
   //    mesh.indices.size()
   //  );
   //  glDepthMask(GL_TRUE);

   glDrawArrays(GL_TRIANGLES, 0, 36);
  }
}
