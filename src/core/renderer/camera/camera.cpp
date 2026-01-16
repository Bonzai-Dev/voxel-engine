#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.hpp"

using namespace Core;

namespace Renderer {
  Camera::Camera(const Core::Application &application, const glm::vec3 &cameraPosition, float nearPlane, float farPlane, float fov) :
    position(cameraPosition), nearPlane(nearPlane), farPlane(farPlane), fov(fov), application(application) {
    updateProjection(nearPlane, farPlane, fov);
  }

  void Camera::update() {
    if (application.mouseMoving()) {
      const auto mouseDelta = application.getMouseDelta();
      rotation.y += mouseDelta.x * MouseSensitivity;
      rotation.x = glm::clamp(rotation.x - mouseDelta.y * MouseSensitivity, -89.9f, 89.9f);
    }

    auto inputDirection = glm::zero<glm::vec3>();
    inputDirection.x += application.keyDown(Inputs::KeyboardKey::KeyW, Inputs::Scancode);
    inputDirection.z += application.keyDown(Inputs::KeyboardKey::KeyA, Inputs::Scancode);
    inputDirection.x -= application.keyDown(Inputs::KeyboardKey::KeyS, Inputs::Scancode);
    inputDirection.z -= application.keyDown(Inputs::KeyboardKey::KeyD, Inputs::Scancode);

    inputDirection.y += application.keyDown(Inputs::KeyboardKey::KeySpace, Inputs::Scancode);
    inputDirection.y -= application.keyDown(Inputs::KeyboardKey::KeyLshift, Inputs::Scancode);

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

  // bool Camera::inView(const Core::AABB &boundingBox) {
  //
  // }
  //
  // float Camera::getSignedDistanceToPlane(const glm::vec3 &position) const {
  //   return glm::dot(normal, position) - distance;
  // }

  void Camera::updateProjection(float near, float far, float fieldOfView) {
    nearPlane = near;
    farPlane = far;
    fov = fieldOfView;
    const auto windowWidth = static_cast<float>(application.getWindow()->getWidth());
    const auto windowHeight = static_cast<float>(application.getWindow()->getHeight());
    const float aspectRatio = windowWidth / windowHeight;
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
  }
}
