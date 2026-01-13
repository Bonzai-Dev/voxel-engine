#pragma once
#include <glm/glm.hpp>
#include <core/application.hpp>

namespace Game {
  constexpr inline float MouseSensitivity = 0.25f;

  class Camera {
    public:
      explicit Camera(
        const Core::Application &application, const glm::vec3 &cameraPosition = glm::vec3(0.0f),
        float nearPlane = 0.1f, float farPlane = 1000, float fov = 75
      );

      void updateProjection(float nearPlane, float farPlane, float fov);

      void update();

      const glm::mat4 &getProjectionMatrix() const { return projectionMatrix; }

      const glm::mat4 &getViewMatrix() const { return viewMatrix; }

    private:
      glm::mat4 projectionMatrix = glm::mat4(1.0f);
      glm::mat4 viewMatrix = glm::mat4(1.0f);

      glm::vec3 rotation = glm::vec3(0, 90, 0);
      glm::vec3 position;

      glm::vec3 forward = glm::vec3(0.0f);
      glm::vec3 right = glm::vec3(0.0f);
      glm::vec3 up = glm::vec3(0.0f);

      float nearPlane;
      float farPlane;
      float fov;

      const Core::Application &application;
  };
}
