#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include <core/logger.hpp>
#include <iostream>
#include "blocks/block.hpp"
#include "../world.hpp"
#include "chunk.hpp"

using namespace Logger;
using namespace Game::Blocks;

namespace Game::Terrain {
  Chunk::Chunk() {
    MeshBuilder();
    m_vertexArrayObject = Renderer::createVertexArrayObject();
    m_vertexBufferObject = Renderer::createVertexBufferObject(m_vertexData.data(), sizeof(float) * m_vertexData.size());
    m_elementBufferObject = Renderer::createElementBufferObject(m_indices.data(), sizeof(unsigned int) * m_indices.size());

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  void Chunk::Render() const {
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
  }

  void Chunk::MeshBuilder() {
    for (int blockCount = 0; blockCount < TotalChunkBlocks; blockCount++) {
      // Fills in the X column first then Z then Y
      const int x = blockCount % ChunkSize;
      const int y = blockCount / (ChunkSize * ChunkSize) + MinChunkHeight;
      const int z = (blockCount / ChunkSize) % ChunkSize;
      BlockId blockId = BlockId::Air;

      if (y < 0)
        blockId = BlockId::Dirt;

      const Block block(glm::vec3(x, y, z), blockId);
      AddBlock(block);

      if (blockId == BlockId::Air)
        continue;
    }

    for (int blockCount = 0; blockCount < TotalChunkBlocks; blockCount++) {
      const int x = blockCount % ChunkSize;
      const int y = blockCount / (ChunkSize * ChunkSize) + MinChunkHeight;
      const int z = (blockCount / ChunkSize) % ChunkSize;
      const auto block = GetBlockLocal(glm::ivec3(x, y, z));

      if (block.GetBlockId() == BlockId::Air)
        continue;

      if (FaceVisible(Forward, block.GetPosition()))
        AddFace(block.GetPosition(), Face::Front);

      if (FaceVisible(Backward, block.GetPosition()))
        AddFace(block.GetPosition(), Face::Back);

      if (FaceVisible(Left, block.GetPosition()))
        AddFace(block.GetPosition(), Face::Left);

      if (FaceVisible(Right, block.GetPosition()))
        AddFace(block.GetPosition(), Face::Right);

      if (FaceVisible(Up, block.GetPosition()))
        AddFace(block.GetPosition(), Face::Top);

      if (FaceVisible(Down, block.GetPosition()))
        AddFace(block.GetPosition(), Face::Bottom);
    }
  }

  void Chunk::AddFace(const glm::ivec3 &position, Face face) {
    const auto &block = GetBlockLocal(position);
    const auto faceIndex = static_cast<unsigned int>(face);
    const auto &vertexData = block.GetMesh().vertexData;

    const unsigned int vertexIndexStart = faceIndex * 4;
    for (unsigned int vertexIndex = vertexIndexStart; vertexIndex < vertexIndexStart + 4; vertexIndex++) {
      const auto &vertex = vertexData[vertexIndex];
      m_vertexData.push_back(position.x + vertex.position.x);
      m_vertexData.push_back(position.y + vertex.position.y);
      m_vertexData.push_back(position.z + vertex.position.z);
      m_vertexData.push_back(vertex.uv.x);
      m_vertexData.push_back(vertex.uv.y);

      // std::cout << "BlockId " << vertexData.size() << '\n';
      // std::cout << "UV: (" << vertex.uv.x << ", " << vertex.uv.y << ")\n";

      // Creates indices for the current face
      if (vertexIndex == vertexIndexStart) {
        const size_t index = m_vertexData.size() / 5 - 1;

        // Top triangle
        m_indices.push_back(index + 2);
        m_indices.push_back(index + 3);
        m_indices.push_back(index + 1);

        // Bottom triangle
        m_indices.push_back(index + 3);
        m_indices.push_back(index);
        m_indices.push_back(index + 1);
      }
    }
  }

  Block Chunk::GetBlockLocal(const glm::ivec3 &position) {
    if (OutOfBounds(position))
      return {glm::vec3(position.x, position.y, position.z), BlockId::Air};

    const int index = position.x + (position.y - MinChunkHeight) * ChunkSize * ChunkSize + position.z * ChunkSize;
    return {glm::vec3(position.x, position.y, position.z), static_cast<BlockId>(m_blocks[index])};
  }

  bool Chunk::OutOfBounds(const glm::ivec3 &position) const {
    const bool xOut = position.x < 0 || position.x >= ChunkSize;
    const bool yOut = position.y < MinChunkHeight || position.y >= MaxChunkHeight;
    const bool zOut = position.z < 0 || position.z >= ChunkSize;
    if (xOut || yOut || zOut)
      return true;

    return false;
  }

  bool Chunk::FaceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position) {
    if (GetBlockLocal(position).GetBlockId() == BlockId::Air)
      return false;

    const glm::ivec3 neighbourPosition = position + faceNormal;
    return GetBlockLocal(neighbourPosition).GetBlockId() == BlockId::Air;
  }

  void Chunk::AddBlock(const Block &block) {
    m_blocks.push_back(static_cast<std::uint16_t>(block.GetBlockId()));
  }
}
