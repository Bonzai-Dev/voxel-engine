#pragma once
#include <array>
#include <glm/mat4x4.hpp>
#include <core/renderer/renderer.hpp>

#include "shaders/skybox.hpp"

namespace Game {
  class Skybox {
    public:
      Skybox(const Renderer::MeshData &mesh);

      ~Skybox() = default;

      void render() const;

    private:
      const std::array<std::string_view, 6> faces {
        "./res/images/skybox/right.jpg",
        "./res/images/skybox/left.jpg",
        "./res/images/skybox/top.jpg",
        "./res/images/skybox/bottom.jpg",
        "./res/images/skybox/front.jpg",
        "./res/images/skybox/back.jpg"
      };

      glm::mat4 transform = glm::mat4(1.0f);

      unsigned int texture;

      unsigned int vertexArrayObject;
      unsigned int vertexBuffer;
      unsigned int indexBuffer;

      const Renderer::MeshData &mesh;

      Shader::Skybox shader;
  };
}
