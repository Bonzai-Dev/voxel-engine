#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include "frustum.hpp"
#include "camera.hpp"

namespace Renderer {
  Frustum::Frustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
    update(projectionMatrix, viewMatrix);
  }

  bool Frustum::boundingBoxInView(const AABB &boundingBox) const {
    for (auto &plane : planes) {
      const auto &minimum = boundingBox.getMinimum();
      const auto &maximum = boundingBox.getMaximum();

      const bool outside = plane.getDistanceToPoint(glm::vec3(minimum.x, minimum.y, minimum.z)) > 0 &&
             plane.getDistanceToPoint(glm::vec3(maximum.x, minimum.y, minimum.z)) > 0 &&
             plane.getDistanceToPoint(glm::vec3(minimum.x, maximum.y, minimum.z)) > 0 &&
             plane.getDistanceToPoint(glm::vec3(maximum.x, maximum.y, minimum.z)) > 0 &&
             plane.getDistanceToPoint(glm::vec3(minimum.x, minimum.y, maximum.z)) > 0 &&
             plane.getDistanceToPoint(glm::vec3(maximum.x, minimum.y, maximum.z)) > 0 &&
             plane.getDistanceToPoint(glm::vec3(minimum.x, maximum.y, maximum.z)) > 0 &&
             plane.getDistanceToPoint(glm::vec3(maximum.x, maximum.y, maximum.z)) > 0;

      if (outside)
        return false;
    }

    return true;
  }

  void Frustum::update(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
    const auto matrix = projectionMatrix * viewMatrix;
    const glm::vec4 &row0 = glm::row(matrix, 0);
    const glm::vec4 &row1 = glm::row(matrix, 1);
    const glm::vec4 &row2 = glm::row(matrix, 2);
    const glm::vec4 &row3 = glm::row(matrix, 3);

    auto &leftPlane = planes[0];
    auto &rightPlane = planes[1];
    auto &bottomPlane = planes[2];
    auto &topPlane = planes[3];
    auto &nearPlane = planes[4];
    auto &farPlane = planes[5];

    leftPlane.transformation = glm::normalize(row0 + row3);
    rightPlane.transformation = glm::normalize(row0 - row3);
    bottomPlane.transformation = glm::normalize(row1 + row3);
    topPlane.transformation = glm::normalize(row1 - row3);
    nearPlane.transformation = glm::normalize(row2 + row3);
    farPlane.transformation = glm::normalize(row2 - row3);
  }
}
