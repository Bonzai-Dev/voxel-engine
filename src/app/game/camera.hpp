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

      void UpdateProjection(float nearPlane, float farPlane, float fov);

      void Update();

      const glm::mat4 &GetProjectionMatrix() const { return m_projectionMatrix; }

      const glm::mat4 &GetViewMatrix() const { return m_viewMatrix; }

    private:
      glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
      glm::mat4 m_viewMatrix = glm::mat4(1.0f);

      glm::vec3 m_rotation = glm::vec3(0, 0, 0);
      glm::vec3 m_position;

      glm::vec3 m_forward = glm::vec3(0.0f);
      glm::vec3 m_right = glm::vec3(0.0f);
      glm::vec3 m_up = glm::vec3(0.0f);

      float m_nearPlane;
      float m_farPlane;
      float m_fov;

      const Core::Application &m_application;
  };
}
