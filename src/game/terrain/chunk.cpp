#include <core/renderer/renderer.hpp>
#include <core/logger.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "blocks/block.hpp"
#include "world.hpp"
#include "chunk.hpp"

using namespace Logger;
using namespace Game::Blocks;

namespace Game {
  Chunk::Chunk(const glm::ivec2 &position, std::vector<int> heightMap, World &world) : world(world),
  position(position), heightMap(std::move(heightMap)) {
    modelMatrix = glm::translate(modelMatrix, glm::vec3(this->position.x, 0, this->position.y));
    buildMesh();
  }

  void Chunk::renderBlocks() {
    loadMesh();
    Renderer::drawTriangles(
      blockVertexArrayObject,
      blockVertexBuffer,
      blockIndexBuffer,
      blockIndices.size()
    );
  }

  void Chunk::renderWater() {
    if (waterVertexData.empty())
      return;

    loadMesh();
    Renderer::drawTriangles(
      waterVertexArrayObject,
      waterVertexBuffer,
      waterIndexBuffer,
      waterIndices.size()
    );
  }

  void Chunk::deleteBuffers() const {
    if (!loadedMesh)
      return;

    Renderer::deleteVertexBufferObject(blockVertexBuffer);
    Renderer::deleteElementBufferObject(blockIndexBuffer);
    Renderer::deleteVertexArrayObject(blockVertexArrayObject);

    if (waterVertexData.empty())
      return;

    Renderer::deleteVertexBufferObject(waterVertexBuffer);
    Renderer::deleteElementBufferObject(waterIndexBuffer);
    Renderer::deleteVertexArrayObject(waterVertexArrayObject);
  }

  void Chunk::loadMesh() {
    if (loadedMesh)
      return;

    loadedMesh = true;
    blockVertexArrayObject = Renderer::createVertexArrayObject();
    blockVertexBuffer = Renderer::createVertexBufferObject(blockVertexData.data(),
                                                           sizeof(float) * blockVertexData.size());
    blockIndexBuffer = Renderer::createElementBufferObject(blockIndices.data(),
                                                           sizeof(unsigned int) * blockIndices.size());

    Renderer::setVertexData<std::float_t>(0, 3, 8, false, 0, blockVertexBuffer);
    Renderer::setVertexData<std::float_t>(1, 3, 8, false, 3, blockVertexBuffer);
    Renderer::setVertexData<std::float_t>(2, 2, 8, false, 6, blockVertexBuffer);

    if (waterVertexData.empty())
      return;

    waterVertexArrayObject = Renderer::createVertexArrayObject();
    waterVertexBuffer = Renderer::createVertexBufferObject(waterVertexData.data(),
                                                           sizeof(float) * waterVertexData.size());
    waterIndexBuffer = Renderer::createElementBufferObject(waterIndices.data(),
                                                           sizeof(unsigned int) * waterIndices.size());
    Renderer::setVertexData<std::float_t>(0, 3, 8, false, 0, waterVertexBuffer);
    Renderer::setVertexData<std::float_t>(1, 3, 8, false, 3, waterVertexBuffer);
    Renderer::setVertexData<std::float_t>(2, 2, 8, false, 6, waterVertexBuffer);
  }

  void Chunk::buildMesh() {
    static const size_t chunkSize = Config::getChunkSize();
    static const size_t totalChunkBlocks = Config::getTotalChunkBlocks();
    static const int minChunkHeight = Config::getMinChunkHeight();

    for (size_t blockCount = 0; blockCount < totalChunkBlocks; blockCount++) {
      const int x = blockCount % chunkSize;
      const int y = blockCount / (chunkSize * chunkSize) + minChunkHeight;
      const int z = (blockCount / chunkSize) % chunkSize;
      const auto &localPosition = glm::ivec3(x, y, z);
      const auto noiseHeight = getNoise(glm::ivec2(localPosition.x, localPosition.z));
      const BlockId blockId = world.evaluateBlockType(localPosition.y, noiseHeight);

      addBlock({localPosition, blockId});
    }

    for (size_t blockCount = 0; blockCount < totalChunkBlocks; blockCount++) {
      const int x = blockCount % chunkSize;
      const int y = blockCount / (chunkSize * chunkSize) + minChunkHeight;
      const int z = (blockCount / chunkSize) % chunkSize;
      const auto &position = glm::ivec3(x, y, z);
      const auto blockId = getBlockLocal(position).getBlockId();

      if (blockId == BlockId::Water) {
        if (faceVisible(Up, position))
          addFace(position, Face::Top, waterVertexData, waterIndices);
        if (faceVisible(Down, position))
          addFace(position, Face::Bottom, waterVertexData, waterIndices);
        if (faceVisible(Forward, position))
          addFace(position, Face::Front, waterVertexData, waterIndices);
        if (faceVisible(Backward, position))
          addFace(position, Face::Back, waterVertexData, waterIndices);
        if (faceVisible(Left, position))
          addFace(position, Face::Left, waterVertexData, waterIndices);
        if (faceVisible(Right, position))
          addFace(position, Face::Right, waterVertexData, waterIndices);
      }

      if (blockId != BlockId::Water && blockId != BlockId::Air) {
        if (faceVisible(Up, position))
          addFace(position, Face::Top, blockVertexData, blockIndices);
        if (faceVisible(Down, position))
          addFace(position, Face::Bottom, blockVertexData, blockIndices);
        if (faceVisible(Forward, position))
          addFace(position, Face::Front, blockVertexData, blockIndices);
        if (faceVisible(Backward, position))
          addFace(position, Face::Back, blockVertexData, blockIndices);
        if (faceVisible(Left, position))
          addFace(position, Face::Left, blockVertexData, blockIndices);
        if (faceVisible(Right, position))
          addFace(position, Face::Right, blockVertexData, blockIndices);
      }
    }
  }

  void Chunk::addFace(const glm::ivec3 &position, Face face, std::vector<float> &vertexData,
                      std::vector<unsigned int> &indices) {
    const auto block = getBlockLocal(position);

    if (block.getBlockId() == BlockId::Air)
      return;

    const auto faceIndex = static_cast<unsigned int>(face);
    const BlockVertexData &blockVertexData = world.getBlockManager().getBlockMeshData(block.getBlockId()).vertexData;
    const glm::vec3 normal = FaceNormals[faceIndex];

    const unsigned int vertexIndexStart = faceIndex * 4;
    for (unsigned int vertexIndex = vertexIndexStart; vertexIndex < vertexIndexStart + 4; vertexIndex++) {
      const BlockVertex &blockVertex = blockVertexData[vertexIndex];
      vertexData.push_back(blockVertex.position.x + position.x);
      vertexData.push_back(blockVertex.position.y + position.y);
      vertexData.push_back(blockVertex.position.z + position.z);

      vertexData.push_back(normal.x);
      vertexData.push_back(normal.y);
      vertexData.push_back(normal.z);

      vertexData.push_back(blockVertex.uv.x);
      vertexData.push_back(blockVertex.uv.y);

      // Creates indices for the current face
      if (vertexIndex == vertexIndexStart) {
        const size_t index = vertexData.size() / 8 - 1;

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
    if (outOfBounds(position)) {
      const glm::vec3 worldPosition = glm::vec3(
        position.x + this->position.x,
        position.y,
        position.z + this->position.y
      );
      return {glm::vec3(position.x, position.y, position.z), world.getBlockId(worldPosition)};
    }

    const size_t index = getBlockIndex(position);
    return {glm::vec3(position.x, position.y, position.z), static_cast<BlockId>(blocks[index])};
  }

  bool Chunk::outOfBounds(const glm::ivec3 &position) {
    static const size_t chunkSize = Config::getChunkSize();
    static const int minChunkHeight = Config::getMinChunkHeight();
    static const int maxChunkHeight = Config::getMaxChunkHeight();

    const bool xOut = position.x < 0 || position.x >= chunkSize;
    const bool yOut = position.y < minChunkHeight || position.y >= maxChunkHeight;
    const bool zOut = position.z < 0 || position.z >= chunkSize;
    if (xOut || yOut || zOut)
      return true;

    return false;
  }

  bool Chunk::faceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position) {
    const auto blockId = getBlockLocal(position).getBlockId();
    const glm::ivec3 neighbourPosition = position + faceNormal;
    const auto neighbourBlockId = getBlockLocal(neighbourPosition).getBlockId();

    if (neighbourBlockId == BlockId::Air)
      return true;

    if (neighbourBlockId == BlockId::Water && blockId != BlockId::Water)
      return true;

    return false;
  }

  void Chunk::addBlock(const Block &block) {
    const size_t index = getBlockIndex(block.getPosition());
    blocks[index] = static_cast<std::uint16_t>(block.getBlockId());
  }

  int Chunk::getNoise(const glm::ivec2 &position) const {
    return heightMap[position.x + position.y * Config::getChunkSize()];
  }

  size_t Chunk::getBlockIndex(const glm::ivec3 &position) {
    static const size_t chunkSize = Config::getChunkSize();
    return position.x + (position.y - Config::getMinChunkHeight()) * chunkSize * chunkSize + position.z * chunkSize;
  }
}
