#include <thread>
#include <iostream>
#include <core/logger.hpp>
#include "terrain.hpp"

namespace Game {
  Terrain::Terrain(const Renderer::Camera &camera): camera(camera) {
    Logger::logInfo(Logger::Context::Game, "Generating terrain with a seed of %d.", seed);

    shader.use();
    shader.updateTexture(Renderer::loadPng("./res/images/blocks.png"));
    for (size_t chunkIndex = 0; chunkIndex < RenderDistance * RenderDistance; chunkIndex++) {
      const int x = (chunkIndex % RenderDistance) * ChunkSize;
      const int z = ((chunkIndex / RenderDistance) % RenderDistance) * ChunkSize;
      const glm::ivec2 position = glm::ivec2(x, z);

      std::thread builder([this, position](){ this->loadChunk(position); });
      chunkBuilder.push_back(std::move(builder));
    }

    joinThreads();

    for (auto &chunk : chunks){
      chunk.loadMesh();
    }

    Logger::logInfo(Logger::Context::Game, "Terrain generated successfully.");
  }

  Terrain::~Terrain() {
    joinThreads();
  }

  void Terrain::joinThreads() {
    for (auto &thread : chunkBuilder) {
      if (thread.joinable())
        thread.join();
    }
  }

  void Terrain::render() {
    shader.use();
    shader.updateProjectionMatrix(camera.getProjectionMatrix());
    shader.updateViewMatrix(camera.getViewMatrix());

    for (auto &chunk : chunks){
      shader.updateModelMatrix(chunk.getModelMatrix());
      chunk.render();
    }
  }

  void Terrain::loadChunk(const glm::ivec2 &position) {
    std::lock_guard lock(chunkMutex);
    chunks.emplace_back(position, generateHeightMap(position / ChunkSize));
  }

  void Terrain::loadTerrain() {
    for (size_t chunkIndex = 0; chunkIndex < RenderDistance * RenderDistance; chunkIndex++) {
      const int chunkX = (chunkIndex / RenderDistance) * ChunkSize;
      const int chunkZ = (chunkIndex % RenderDistance) * ChunkSize;
      const glm::ivec2 position = glm::ivec2(chunkX, chunkZ);
      loadChunk(position);
    }
  }

  std::vector<int> Terrain::generateHeightMap(const glm::ivec2 &position) {
    std::vector<int> heightMap;
    for (size_t blockCount = 0; blockCount < ChunkSize * ChunkSize; blockCount++) {
      const int x = blockCount % ChunkSize + (position.x * ChunkSize);
      const int z = (blockCount / ChunkSize) % ChunkSize + (position.y * ChunkSize);
      const int y = noise2d(glm::ivec2(x, z), glm::vec2(0.01, 0.01), 50);
      heightMap.push_back(y);
    }
    return heightMap;
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
