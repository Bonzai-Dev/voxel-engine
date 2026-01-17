#include <glm/gtc/matrix_access.hpp>
#include "frustum.hpp"
#include "camera.hpp"

namespace Renderer {
  Frustum::Frustum(const glm::mat4 &projectionMartix) {
    update(projectionMartix);
  }

  void Frustum::update(const glm::mat4 &projectionMatrix) {
    const auto &row0 = glm::row(projectionMatrix, 0);
    const auto &row1 = glm::row(projectionMatrix, 1);
    const auto &row2 = glm::row(projectionMatrix, 2);
    const auto &row3 = glm::row(projectionMatrix, 3);

    const glm::vec4 leftPlane = glm::normalize(row0 + row3);
    const glm::vec4 rightPlane = glm::normalize(row0 - row3);
    const glm::vec4 bottomPlane = glm::normalize(row1 + row3);
    const glm::vec4 topPlane = glm::normalize(row1 - row3);
    const glm::vec4 nearPlane = glm::normalize(row2 + row3);
    const glm::vec4 farPlane = glm::normalize(row2 - row3);

    left.normal = glm::vec3(leftPlane);
    left.distance = leftPlane.w;

    right.normal = glm::vec3(rightPlane);
    right.distance = rightPlane.w;

    bottom.normal = glm::vec3(bottomPlane);
    bottom.distance = bottomPlane.w;

    top.normal = glm::vec3(topPlane);
    top.distance = topPlane.w;

    near.normal = glm::vec3(nearPlane);
    near.distance = nearPlane.w;

    far.normal = glm::vec3(farPlane);
    far.distance = farPlane.w;
  }
}
