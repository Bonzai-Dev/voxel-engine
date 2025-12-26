#pragma once

#include <glm/glm.hpp>
#include <core/application.hpp>

namespace Game {
  class Camera {
    public:
      explicit Camera(
        const Core::Application &application, const glm::vec3 &cameraPosition = glm::vec3(0.0f),
        float nearPlane = 0.1f, float farPlane = 1000, float fov = 75
      );

      void UpdateProjection(float nearPlane, float farPlane, float fov);

      void Update();

      glm::mat4 projectionMatrix = glm::mat4(1.0f);
      glm::mat4 viewMatrix = glm::mat4(1.0f);

      glm::vec3 rotation = glm::vec3(0, -90, 0);
      glm::vec3 position;

      glm::vec3 forward = glm::vec3(0.0f);
      glm::vec3 right = glm::vec3(0.0f);
      glm::vec3 up = glm::vec3(0.0f);

      float nearPlane;
      float farPlane;
      float fov;

    private:
      const Core::Application &m_application;
  };
}
