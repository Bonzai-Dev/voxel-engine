#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.hpp"

using Core::Inputs::KeyboardKey;

namespace Game {

  Camera::Camera(const Core::Application &application, const glm::vec3 &cameraPosition, float nearPlane, float farPlane, float fov) :
  position(cameraPosition), nearPlane(nearPlane), farPlane(farPlane), fov(fov),
    m_application(application) {
    UpdateProjection(nearPlane, farPlane, fov);
  }

  void Camera::Update() {
    if (m_application.mouseMoving) {
      constexpr float mouseSensitivity = 0.25f;
      rotation.y += m_application.mouseDelta.x * mouseSensitivity;
      rotation.x = glm::clamp(rotation.x - m_application.mouseDelta.y * mouseSensitivity, -89.9f, 89.9f);
    }

    auto inputDirection = glm::zero<glm::vec3>();
    inputDirection.x += m_application.KeyDown(KeyboardKey::KeyW, Core::Inputs::KeyDetectScancode);
    inputDirection.z += m_application.KeyDown(KeyboardKey::KeyA, Core::Inputs::KeyDetectScancode);
    inputDirection.x -= m_application.KeyDown(KeyboardKey::KeyS, Core::Inputs::KeyDetectScancode);
    inputDirection.z -= m_application.KeyDown(KeyboardKey::KeyD, Core::Inputs::KeyDetectScancode);

    inputDirection.y += m_application.KeyDown(KeyboardKey::KeySpace, Core::Inputs::KeyDetectScancode);
    inputDirection.y -= m_application.KeyDown(KeyboardKey::KeyLshift, Core::Inputs::KeyDetectScancode);

    glm::vec3 moveDirection = forward * inputDirection.x + right * inputDirection.z;
    moveDirection += glm::vec3(0, inputDirection.y, 0);
    position += moveDirection;

    const glm::vec3 direction(
      cosf(glm::radians(rotation.y)) * cosf(glm::radians(rotation.x)),
      sinf(glm::radians(rotation.x)),
      sinf(glm::radians(rotation.y)) * cosf(glm::radians(rotation.x))
    );

    forward = glm::normalize(direction);
    right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));
    up = glm::normalize(glm::cross(forward, right));

    viewMatrix = glm::lookAt(
      position,
      position + forward,
      up
    );
  }

  void Camera::UpdateProjection(float near, float far, float fieldOfView) {
    nearPlane = near;
    farPlane = far;
    fov = fieldOfView;
    const auto windowWidth = static_cast<float>(m_application.window->GetWidth());
    const auto windowHeight = static_cast<float>(m_application.window->GetHeight());
    const float aspectRatio = windowWidth / windowHeight;
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
  }
}
