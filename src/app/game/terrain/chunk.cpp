#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include "blocks/air.hpp"
#include "core/logger.hpp"
#include "../world.hpp"
#include "chunk.hpp"

#include <iostream>

using namespace Logger;

static constexpr std::array frontFace{
  glm::vec3(0, 0, 1), glm::vec3(1, 0, 1), glm::vec3(1, 1, 1), glm::vec3(0, 1, 1),
};

static constexpr std::array backFace{
  glm::vec3(1, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 1, 0),
};

static constexpr std::array leftFace{
  glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 1), glm::vec3(0, 1, 0),
};

static constexpr std::array rightFace{
  glm::vec3(1, 0, 1),
  glm::vec3(1, 0, 0),
  glm::vec3(1, 1, 0),
  glm::vec3(1, 1, 1)
};

static constexpr std::array topFace{
  glm::vec3(0, 1, 1),
  glm::vec3(1, 1, 1),
  glm::vec3(1, 1, 0),
  glm::vec3(0, 1, 0),
};

static constexpr std::array bottomFace{
  glm::vec3(0, 0, 0),
  glm::vec3(1, 0, 0),
  glm::vec3(1, 0, 1),
  glm::vec3(0, 0, 1)
};

static std::vector<glm::vec3> faceVertices;

namespace Game {
  Chunk::Chunk() {
    MeshBuilder();
    m_vertexArrayObject = Renderer::createVertexArrayObject();
    m_vertexBufferObject = Renderer::createVertexBufferObject(m_vertexData.data(), sizeof(float) * m_vertexData.size());
    m_elementBufferObject = Renderer::createElementBufferObject(m_indices.data(), sizeof(unsigned int) * m_indices.size());

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    Renderer::setFillMode(Renderer::MeshFillMode::Wireframe);
  }

  void Chunk::Render() const {
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
    // glDrawArrays(GL_TRIANGLES, 0, m_vertexData.size() / 3);
  }

  void Chunk::MeshBuilder() {
    // TODO: TEMPORARY - REMOVE THIS LATER
    for (unsigned int faceIndex = 0; faceIndex < 6; faceIndex++) {
      const auto face = static_cast<Face>(faceIndex);
      switch (face) {
        case Face::Front:
          faceVertices.insert(faceVertices.end(), frontFace.begin(), frontFace.end());
          break;
        case Face::Back:
          faceVertices.insert(faceVertices.end(), backFace.begin(), backFace.end());
          break;
        case Face::Left:
          faceVertices.insert(faceVertices.end(), leftFace.begin(), leftFace.end());
          break;
        case Face::Right:
          faceVertices.insert(faceVertices.end(), rightFace.begin(), rightFace.end());
          break;
        case Face::Top:
          faceVertices.insert(faceVertices.end(), topFace.begin(), topFace.end());
          break;
        case Face::Bottom:
          faceVertices.insert(faceVertices.end(), bottomFace.begin(), bottomFace.end());
          break;
      }
    }

    for (int blockCount = 0; blockCount < TotalChunkBlocks; blockCount++) {
      // Fills in the X column first then Z then Y
      const int x = blockCount % ChunkSize;
      const int y = blockCount / (ChunkSize * ChunkSize) + MinChunkHeight;
      const int z = (blockCount / ChunkSize) % ChunkSize;
      const auto blockPosition = glm::vec3(x, y, z);
      BlockId blockId = BlockId::Air;

      if (y < 0)
        blockId = BlockId::Dirt;

      AddBlockId(blockId);

      if (blockId == BlockId::Air)
        continue;
      //
      // AddFace(blockPosition, Face::Front);
      // AddFace(blockPosition, Face::Back);
      // AddFace(blockPosition, Face::Left);
      // AddFace(blockPosition, Face::Right);
      // AddFace(blockPosition, Face::Top);
      // AddFace(blockPosition, Face::Bottom);
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
    const unsigned int faceStartIndex = static_cast<unsigned int>(face) * 4;
    for (unsigned int vertexIndex = faceStartIndex; vertexIndex < faceStartIndex + 4; vertexIndex++) {
      const auto vertexData = faceVertices[vertexIndex];
      m_vertexData.push_back(position.x + vertexData.x);
      m_vertexData.push_back(position.y + vertexData.y);
      m_vertexData.push_back(position.z + vertexData.z);

      if (vertexIndex == faceStartIndex) {
        const size_t index = m_vertexData.size() / 3 - 1;

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
    if (!PositionInChunk(position))
      return Air(glm::vec3(position.x, position.y, position.z));

    const int index = position.x + (position.y - MinChunkHeight) * ChunkSize * ChunkSize + position.z * ChunkSize;
    return {glm::vec3(position.x, position.y, position.z), static_cast<BlockId>(m_blocks.at(index))};
  }

  bool Chunk::PositionInChunk(const glm::ivec3 &position) const {
    const bool xOut = position.x < 0 || position.x >= ChunkSize;
    const bool yOut = position.y < MinChunkHeight || position.y >= MaxChunkHeight;
    const bool zOut = position.z < 0 || position.z >= ChunkSize;
    if (xOut || yOut || zOut)
      return false;

    return true;
  }

  bool Chunk::FaceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position) {
    if (GetBlockLocal(position).GetBlockId() == BlockId::Air)
      return false;

    const glm::ivec3 neighbourPosition = position + faceNormal;
    return GetBlockLocal(neighbourPosition).GetBlockId() == BlockId::Air;
  }

  void Chunk::AddBlockId(BlockId block) {
    m_blocks.push_back(static_cast<std::uint16_t>(block));
  }
}
