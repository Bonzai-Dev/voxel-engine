#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.hpp"
#include "config.hpp"

using namespace Core;

namespace Game {
  Camera::Camera(const Application &application, const glm::vec3 &cameraPosition, float far, float fov, float near) :
  position(cameraPosition),
  aspectRatio(application.getWindow()->getWidth() / application.getWindow()->getHeight()),
  fov(fov), application(application), farDistance(far), nearDistance(near) {
    updateProjection(near, far, fov);
  }

  void Camera::update() {
    if (application.mouseMoving()) {
      const auto mouseDelta = application.getMouseDelta();
      rotation.y += mouseDelta.x * Config::getCameraSensitivity();
      rotation.x = glm::clamp(rotation.x - mouseDelta.y * Config::Config::getCameraSensitivity(), -89.9f, 89.9f);
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
    position += moveDirection * static_cast<float>(application.getDeltaTime()) * Config::getCameraSpeed();
    isMoving = glm::length(moveDirection) != 0;

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

  void Camera::updateProjection(float near, float far, float fov) {
    this->fov = fov;
    nearDistance = near;
    farDistance = far;
    const auto windowWidth = static_cast<float>(application.getWindow()->getWidth());
    const auto windowHeight = static_cast<float>(application.getWindow()->getHeight());
    const float aspectRatio = windowWidth / windowHeight;
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearDistance, farDistance);
  }
}
