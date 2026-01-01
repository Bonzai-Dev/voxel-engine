#include "dirt.hpp"

namespace Game {
  Dirt::Dirt(const glm::vec3 &position) : Block(position, BlockId::Dirt) {
    LoadBlockData("./res/data/blocks/dirt.json");
    GenerateVertexData();
  }
}
