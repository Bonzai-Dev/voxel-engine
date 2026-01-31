#include <util/io.hpp>
#include <glm/detail/func_geometric.inl>
#include "block_data.hpp"
#include "block.hpp"

using namespace Logger;

namespace Game::Blocks {
  Block::Block(const glm::vec3 &position, BlockId blockId) : blockId(blockId), position(position), meshId(MeshId::Block) {
  }
}
