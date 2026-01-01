#pragma once
#include "block.hpp"

namespace Game {
  class Air: public Block {
    public:
      explicit Air(const glm::vec3 &position);
  };
}