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

      void render(const Camera &camera) const;

    private:
      const std::array<std::string_view, 6> faces {
        "./res/images/skybox/right.png",
        "./res/images/skybox/left.png",
        "./res/images/skybox/top.png",
        "./res/images/skybox/bottom.png",
        "./res/images/skybox/front.png",
        "./res/images/skybox/back.png"
      };

      glm::mat4 transform = glm::mat4(1.0f);

      unsigned int cubeMap;

      unsigned int vertexArrayObject;
      unsigned int vertexBuffer;
      unsigned int indexBuffer;

      const Renderer::MeshData &mesh;

      Shader::Skybox shader;
  };
}
