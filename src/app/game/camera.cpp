#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.hpp"

using namespace Core;

namespace Game {
  Camera::Camera(const Application &application, const glm::vec3 &cameraPosition, float nearPlane, float farPlane, float fov) :
    m_position(cameraPosition), m_nearPlane(nearPlane), m_farPlane(farPlane), m_fov(fov), m_application(application) {
    UpdateProjection(nearPlane, farPlane, fov);
  }

  void Camera::Update() {
    if (m_application.mouseMoving) {
      m_rotation.y += m_application.mouseDelta.x * MouseSensitivity;
      m_rotation.x = glm::clamp(m_rotation.x - m_application.mouseDelta.y * MouseSensitivity, -89.9f, 89.9f);
    }

    auto inputDirection = glm::zero<glm::vec3>();
    inputDirection.x += m_application.KeyDown(Inputs::KeyboardKey::KeyW, Inputs::Scancode);
    inputDirection.z += m_application.KeyDown(Inputs::KeyboardKey::KeyA, Inputs::Scancode);
    inputDirection.x -= m_application.KeyDown(Inputs::KeyboardKey::KeyS, Inputs::Scancode);
    inputDirection.z -= m_application.KeyDown(Inputs::KeyboardKey::KeyD, Inputs::Scancode);

    inputDirection.y += m_application.KeyDown(Inputs::KeyboardKey::KeySpace, Inputs::Scancode);
    inputDirection.y -= m_application.KeyDown(Inputs::KeyboardKey::KeyLshift, Inputs::Scancode);

    glm::vec3 moveDirection = m_forward * inputDirection.x + m_right * inputDirection.z;
    moveDirection += glm::vec3(0, inputDirection.y, 0);
    m_position += moveDirection;

    const glm::vec3 direction(
      cosf(glm::radians(m_rotation.y)) * cosf(glm::radians(m_rotation.x)),
      sinf(glm::radians(m_rotation.x)),
      sinf(glm::radians(m_rotation.y)) * cosf(glm::radians(m_rotation.x))
    );

    m_forward = glm::normalize(direction);
    m_right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), m_forward));
    m_up = glm::normalize(glm::cross(m_forward, m_right));

    m_viewMatrix = glm::lookAt(
      m_position,
      m_position + m_forward,
      m_up
    );
  }

  void Camera::UpdateProjection(float near, float far, float fieldOfView) {
    m_nearPlane = near;
    m_farPlane = far;
    m_fov = fieldOfView;
    const auto windowWidth = static_cast<float>(m_application.window->GetWidth());
    const auto windowHeight = static_cast<float>(m_application.window->GetHeight());
    const float aspectRatio = windowWidth / windowHeight;
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
  }
}
