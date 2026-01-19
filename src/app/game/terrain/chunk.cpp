#include <thread>
#include <core/renderer/renderer.hpp>
#include <core/logger.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "blocks/block.hpp"
#include "terrain.hpp"
#include "chunk.hpp"

using namespace Logger;
using namespace Game::Blocks;

namespace Game {
  Chunk::Chunk(const glm::ivec2 &position, const std::vector<int> &heightMap):
  position(position), heightMap(heightMap) {
    modelMatrix = glm::translate(modelMatrix, glm::vec3(this->position.x, 0, this->position.y));
    auto meshBuilder = std::thread(&Chunk::buildMesh, this);
    meshBuilder.join();

    vertexArrayObject = Renderer::createVertexArrayObject();
    vertexBufferObject = Renderer::createVertexBufferObject(vertexData.data(), sizeof(float) * vertexData.size());
    elementBufferObject = Renderer::createElementBufferObject(indices.data(), sizeof(unsigned int) * indices.size());

    Renderer::setVertexData<std::float_t>(0, 3, 5, false, 0, vertexBufferObject);
    Renderer::setVertexData<std::float_t>(1, 2, 5, false, 3, vertexBufferObject);
  }

  void Chunk::render() const {
    Renderer::drawTriangles(vertexArrayObject, vertexBufferObject, elementBufferObject, indices.size());
  }

  void Chunk::buildMesh() {
    for (size_t blockCount = 0; blockCount < TotalChunkBlocks; blockCount++) {
      const int x = blockCount % ChunkSize;
      const int y = blockCount / (ChunkSize * ChunkSize) + MinChunkHeight;
      const int z = (blockCount / ChunkSize) % ChunkSize;
      const auto &localPosition = glm::ivec3(x, y, z);
      const auto noiseHeight = getNoise(glm::ivec2(localPosition.x, localPosition.z));

      BlockId blockId = BlockId::Air;
      if (y == noiseHeight)
        blockId = BlockId::Grass;

      if (y < noiseHeight)
        blockId = BlockId::Dirt;

      if (y == MinChunkHeight)
        blockId = BlockId::Bedrock;

      addBlock({localPosition, blockId});
    }

    for (size_t blockCount = 0; blockCount < TotalChunkBlocks; blockCount++) {
      const int x = blockCount % ChunkSize;
      const int y = blockCount / (ChunkSize * ChunkSize) + MinChunkHeight;
      const int z = (blockCount / ChunkSize) % ChunkSize;
      const auto &position = glm::ivec3(x, y, z);

      if (faceVisible(Up, position))
        addFace(position, Face::Top);
      if (faceVisible(Down, position))
        addFace(position, Face::Bottom);
      if (faceVisible(Forward, position))
        addFace(position, Face::Front);
      if (faceVisible(Backward, position))
        addFace(position, Face::Back);
      if (faceVisible(Left, position))
        addFace(position, Face::Left);
      if (faceVisible(Right, position))
        addFace(position, Face::Right);
    }
  }

  void Chunk::addFace(const glm::ivec3 &position, Face face) {
    const auto block = getBlockLocal(position);

    if (block.getBlockId() == BlockId::Air)
      return;

    const auto faceIndex = static_cast<unsigned int>(face);
    const auto &blockVertexData = block.getMesh().vertexData;

    const unsigned int vertexIndexStart = faceIndex * 4;
    for (unsigned int vertexIndex = vertexIndexStart; vertexIndex < vertexIndexStart + 4; vertexIndex++) {
      const auto &blockVertex = blockVertexData[vertexIndex];

      vertexData.push_back(blockVertex.position.x + position.x);
      vertexData.push_back(blockVertex.position.y + position.y);
      vertexData.push_back(blockVertex.position.z + position.z);

      vertexData.push_back(blockVertex.uv.x);
      vertexData.push_back(blockVertex.uv.y);

      // Creates indices for the current face
      if (vertexIndex == vertexIndexStart) {
        const size_t index = vertexData.size() / 5 - 1;

        // Top triangle
        indices.push_back(index + 2);
        indices.push_back(index + 3);
        indices.push_back(index + 1);

        // Bottom triangle
        indices.push_back(index + 3);
        indices.push_back(index);
        indices.push_back(index + 1);
      }
    }
  }

  Block Chunk::getBlockLocal(const glm::ivec3 &position) {
    if (outOfBounds(position))
      return {glm::vec3(position.x, position.y, position.z), BlockId::Air};

    const size_t index = getBlockIndex(position);
    return {glm::vec3(position.x, position.y, position.z), static_cast<BlockId>(blocks[index])};
  }

  bool Chunk::outOfBounds(const glm::ivec3 &position) {
    const bool xOut = position.x < 0 || position.x >= ChunkSize;
    const bool yOut = position.y < MinChunkHeight || position.y >= MaxChunkHeight;
    const bool zOut = position.z < 0 || position.z >= ChunkSize;
    if (xOut || yOut || zOut)
      return true;

    return false;
  }

  bool Chunk::faceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position) {
    const glm::ivec3 neighbourPosition = position + faceNormal;
    return getBlockLocal(neighbourPosition).getBlockId() == BlockId::Air;
  }

  void Chunk::addBlock(const Block &block) {
    const size_t index = getBlockIndex(block.getPosition());
    blocks[index] = static_cast<std::uint16_t>(block.getBlockId());
  }

  int Chunk::getNoise(const glm::ivec2 &position) const {
    return heightMap[position.x + position.y * ChunkSize];
  }

  size_t Chunk::getBlockIndex(const glm::ivec3 &position) {
    return position.x + (position.y - MinChunkHeight) * ChunkSize * ChunkSize + position.z * ChunkSize;
  }
}
