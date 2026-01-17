#pragma once
#include "frustum_plane.hpp"

namespace Renderer {
  class Camera;

  class Frustum {
    public:
      Frustum(const glm::mat4 &projectionMatrix);

      const FrustumPlane &getLeftPlane() const { return left; }

      const FrustumPlane &getRightPlane() const { return right; }

      const FrustumPlane &getTopPlane() const { return top; }

      const FrustumPlane &getBottomPlane() const { return bottom; }

      const FrustumPlane &getNearPlane() const { return near; }

      const FrustumPlane &getFarPlane() const { return far; }

      void update(const glm::mat4 &projectionMatrix);

    private:
      FrustumPlane left;
      FrustumPlane right;
      FrustumPlane top;
      FrustumPlane bottom;
      FrustumPlane near;
      FrustumPlane far;
  };
}