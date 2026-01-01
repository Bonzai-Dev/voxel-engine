#pragma once
#include "block.hpp"

namespace Game {
  class Dirt: Block {
    public:
      explicit Dirt(const glm::vec3 &position);
  };
}
