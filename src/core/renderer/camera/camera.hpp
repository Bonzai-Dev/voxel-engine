#pragma once
#include <glm/glm.hpp>
#include <core/application/application.hpp>
#include "frustum.hpp"
#include "core/renderer/aabb.hpp"

namespace Renderer {
  constexpr inline float MouseSensitivity = 0.25f;

  class Camera {
    public:
      explicit Camera(
        const Core::Application &application, const glm::vec3 &cameraPosition = glm::vec3(0.0f),
        float fov = 75, float near = 0.1f, float far = 10000000
      );

      void updateProjection(float near, float far, float fov);

      void update();

      const glm::mat4 &getProjectionMatrix() const { return projectionMatrix; }

      const glm::mat4 &getViewMatrix() const { return viewMatrix; }

      const glm::vec3 &getForwardDirection() const { return forward; }

      const glm::vec3 &getRightDirection() const { return right; }

      const glm::vec3 &getUpDirection() const { return up; }

      const glm::vec3 &getPosition() const { return position; }

      const bool inView(const AABB &boundingBox) const { return frustum.boundingBoxInView(boundingBox); }

      bool moving() const { return isMoving; }

    private:
      glm::mat4 projectionMatrix = glm::mat4(1.0f);
      glm::mat4 viewMatrix = glm::mat4(1.0f);

      glm::vec3 rotation = glm::vec3(0, 90, 0);
      glm::vec3 position{};

      glm::vec3 forward{};
      glm::vec3 right{};
      glm::vec3 up{};

      float nearDistance{};
      float farDistance{};

      float aspectRatio;
      float fov;

      bool isMoving;

      Frustum frustum;

      const Core::Application &application;
  };
}
