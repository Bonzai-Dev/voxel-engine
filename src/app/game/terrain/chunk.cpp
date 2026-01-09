#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include <core/logger.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <core/open_simplex2s.hpp>
#include "blocks/block.hpp"
#include "../world.hpp"
#include "chunk.hpp"

using namespace Logger;
using namespace Game::Blocks;

namespace Game::Terrain {
  Chunk::Chunk(const glm::ivec2 &position): m_position(position * ChunkSize) {
    m_modelMatrix = glm::translate(m_modelMatrix, glm::vec3(m_position.x, 0, m_position.y));
    std::cout << "Generating chunk at (" << m_position.x << ", " << m_position.y << ")\n";
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
    static int chunkCount = 0;
    for (size_t blockCount = 0; blockCount < TotalChunkBlocks; blockCount++) {
      // Fills in the X column first then Z then Y
      const int x = blockCount % ChunkSize;
      const int y = blockCount / (ChunkSize * ChunkSize) + MinChunkHeight;
      const int z = (blockCount / ChunkSize) % ChunkSize;
      const auto globalX = static_cast<double>(m_position.x + x);
      const auto globalZ = static_cast<double>(m_position.y + z);
      constexpr auto noiseFrequency = 0.05f;
      const int noiseHeight = (int) floor((sinf(globalX * 0.3f) + cosf(globalZ * 0.3f)) * 3.0f);

      // int noiseHeight = roundf(m_noise.eval(globalX * noiseFrequency, globalZ * noiseFrequency) * 50.0f);
      BlockId blockId = BlockId::Air;

      // if (chunkCount == 0 && x == ChunkSize - 1 && z == ChunkSize - 1 && y == noiseHeight) {
      //   std::cout << "Noise at (" << globalX << ", " << globalZ << "): " << noiseHeight << "\n";
      // }
      //
      // if (chunkCount == 1 && x == 0 && z == ChunkSize - 1 && y == noiseHeight) {
      //   std::cout << "Noise at (" << globalX << ", " << globalZ << "): " << noiseHeight << "\n";
      // }
      //
      // if (chunkCount == 2 && x == 0 && z == ChunkSize - 1 && y == noiseHeight) {
      //   std::cout << "Noise at (" << globalX << ", " << globalZ << "): " << noiseHeight << "\n";
      // }

      if (y == noiseHeight) {
      std::cout << "Added block " << " at position " << x << " " << y << " " << z << "\n";
        std::cout << "Set grass at (" << globalX << ", " << y << ", " << globalZ << ")" << " Noise: " << noiseHeight << "\n";
        blockId = BlockId::Grass;
      }

      if (y < noiseHeight)
        blockId = BlockId::Dirt;

      // if (y == MinChunkHeight)
      //   blockId = BlockId::Bedrock;
      //
      if (blockId == BlockId::Air)
        continue;

      const Block block(glm::vec3(x, y, z), blockId);
      AddBlock(block);
    }
    chunkCount ++;
    for (size_t blockCount = 0; blockCount < TotalChunkBlocks; blockCount++) {
      const int x = blockCount % ChunkSize;
      const int y = blockCount / (ChunkSize * ChunkSize) + MinChunkHeight;
      const int z = (blockCount / ChunkSize) % ChunkSize;
      const int globalX = x + m_position.x;
      const int globalZ = z + m_position.y;
      const auto block = GetBlockLocal(glm::ivec3(x, y, z));
      // const int noiseHeight = (int) floor((sinf(globalX * 0.3f) + cosf(globalZ * 0.3f)) * 3.0f);
      // const auto position = glm::ivec3(x, noiseHeight, z);
      const auto &position = block.GetPosition();

      if (FaceVisible(Up, position))
        AddFace(position, Face::Top);
      if (FaceVisible(Down, position))
        AddFace(position, Face::Bottom);
      if (FaceVisible(Forward, position))
        AddFace(position, Face::Front);
      if (FaceVisible(Backward, position))
        AddFace(position, Face::Back);
      if (FaceVisible(Left, position))
        AddFace(position, Face::Left);
      if (FaceVisible(Right, position))
        AddFace(position, Face::Right);
    }
  }

  void Chunk::AddFace(const glm::ivec3 &position, Face face) {
    const auto block = GetBlockLocal(position);
    const auto globalX = static_cast<double>(m_position.x + block.GetPosition().x);
    const auto globalZ = static_cast<double>(m_position.y + block.GetPosition().z);
    int noiseHeight = static_cast<int>(roundf(std::sinf(globalX * 0.05f) * std::cosf(globalZ * 0.05f) * 20.0f));

    if (block.GetBlockId() == BlockId::Air)
      return;

    //
    // if (block.GetPosition().y == noiseHeight) {
    //   std::cout << "added face at (" << globalX << ", " << block.GetPosition().y << ", " << globalZ << ")" << " Noise: " << noiseHeight << "\n";
    // }

    // const int x = static_cast<int>(block.GetPosition().x);
    // const int y = static_cast<int>(block.GetPosition().y);
    // const int z = static_cast<int>(block.GetPosition().z);
    // const int chunkCount = index;
    // if (chunkCount == 0 && x == ChunkSize - 1 && z == ChunkSize - 1 && y == noiseHeight) {
    //   std::cout << "Noise at (" << globalX << ", " << globalZ << "): " << noiseHeight << "\n";
    // }
    //
    // if (chunkCount == 1 && x == 0 && z == ChunkSize - 1 && y == noiseHeight) {
    //   std::cout << "Noise at (" << globalX << ", " << globalZ << "): " << noiseHeight << "\n";
    // }
    //
    // if (chunkCount == 2 && x == 0 && z == ChunkSize - 1 && y == noiseHeight) {
    //   std::cout << "Noise at (" << globalX << ", " << globalZ << "): " << noiseHeight << "\n";
    // }


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

    const size_t index = GetBlockIndex(position);
    return {glm::vec3(position.x, position.y, position.z), static_cast<BlockId>(m_blocks[index])};
  }

  bool Chunk::OutOfBounds(const glm::ivec3 &position) {
    const bool xOut = position.x < 0 || position.x >= ChunkSize;
    const bool yOut = position.y < MinChunkHeight || position.y >= MaxChunkHeight;
    const bool zOut = position.z < 0 || position.z >= ChunkSize;
    if (xOut || yOut || zOut)
      return true;

    return false;
  }

  bool Chunk::FaceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position) {
    const glm::ivec3 neighbourPosition = position + faceNormal;
    return GetBlockLocal(neighbourPosition).GetBlockId() == BlockId::Air;
  }

  void Chunk::AddBlock(const Block &block) {
    const size_t index = GetBlockIndex(block.GetPosition());
    m_blocks[index] = static_cast<std::uint16_t>(block.GetBlockId());
  }

  size_t Chunk::GetBlockIndex(const glm::ivec3 &position) {
    return position.x + (position.y - MinChunkHeight) * ChunkSize * ChunkSize + position.z * ChunkSize;
  }
}
