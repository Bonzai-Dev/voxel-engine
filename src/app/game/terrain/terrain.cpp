#include <thread>
#include <iostream>
#include <core/logger.hpp>
#include "terrain.hpp"

namespace Game {
  Terrain::Terrain(const Renderer::Camera &camera): camera(camera) {
    Logger::logInfo(Logger::Context::Game, "Generating terrain with a seed of %d.", seed);

    shader.use();
    shader.updateTexture(Renderer::loadPng("./res/images/blocks.png"));

    auto terrainBuilder = std::thread(&Terrain::loadChunks, this);
    terrainBuilder.join();

    for (auto &chunk : chunks) {
      chunk.loadMesh();
    }

    Logger::logInfo(Logger::Context::Game, "Terrain generated successfully.");
  }

  void Terrain::render() {
    shader.use();
    shader.updateProjectionMatrix(camera.getProjectionMatrix());
    shader.updateViewMatrix(camera.getViewMatrix());

    for (size_t index = 0; index < chunks.size(); index++){
      const auto &chunk = chunks[index];
      shader.updateModelMatrix(chunk.getModelMatrix());
      chunk.render();
    }
  }

  // const Chunk &Terrain::getChunk(const glm::ivec2 &position) {
  // }

  void Terrain::loadChunks() {
    const glm::ivec2 cameraPosition = glm::ivec2(camera.getPosition().x, camera.getPosition().z) / ChunkSize;
    // const glm::ivec2 cameraPosition = glm::ivec2(0, 0);
    std::cout << "Camera Position: " <<   cameraPosition.x << ", " << cameraPosition.y << std::endl;

    std::cout << "StartX: " << cameraPosition.x - RenderDistance / 2 << " EndX: " << cameraPosition.x + RenderDistance / 2 << std::endl;
    std::cout << "StartZ: " << cameraPosition.y - RenderDistance / 2 << " EndZ: " << cameraPosition.y + RenderDistance / 2 << std::endl;

    for (int x = cameraPosition.x - RenderDistance / 2; x < cameraPosition.y + RenderDistance / 2; x++) {
      for (int z = cameraPosition.y - RenderDistance / 2; z < cameraPosition.y + RenderDistance / 2; z++) {
        const glm::ivec2 position = glm::ivec2(x, z);
        const auto chunk = Chunk(position * ChunkSize, generateHeightMap(position));
        chunks.push_back(chunk);
      }
    }
  }

  std::vector<int> Terrain::generateHeightMap(const glm::ivec2 &position) const {
    std::vector<int> heightMap;
    for (size_t blockCount = 0; blockCount < ChunkSize * ChunkSize; blockCount++) {
      const int x = blockCount % ChunkSize + (position.x * ChunkSize);
      const int z = (blockCount / ChunkSize) % ChunkSize + (position.y * ChunkSize);
      const int y = noise2d(glm::ivec2(x, z), glm::vec2(0.01, 0.01), 50);
      heightMap.push_back(y);
    }
    return heightMap;
  }

  size_t Terrain::getChunkIndex(const glm::ivec2 &position) {
    const glm::ivec2 indexPosition = (position + ChunkSize * (RenderDistance / 2)) / ChunkSize;
    return indexPosition.x + indexPosition.y * ChunkSize;
  }

  int Terrain::generateSeed() {
    srand(time(nullptr));
    static constexpr long seedRange = static_cast<long>(10e8);
    return rand() % (2 * seedRange + 1) - seedRange;
  }

  int Terrain::noise2d(const glm::ivec2 &position, const glm::vec2 &scale, float amplitude) const {
    const auto x = static_cast<double>(position.x);
    const auto y = static_cast<double>(position.y);
    return static_cast<int>(floor(noiseGenerator.eval(x * scale.x, y * scale.y) * amplitude));
  }
}
