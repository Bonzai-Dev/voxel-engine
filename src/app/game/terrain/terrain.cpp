#include <iostream>
#include <core/logger.hpp>
#include "terrain.hpp"

#include <thread>

namespace Game {
  Terrain::Terrain(const Renderer::Camera &camera): camera(camera) {
    Logger::logInfo(Logger::Context::Game, "Generating terrain with a seed of %d.", seed);

    shader.use();
    shader.updateTexture(Renderer::loadPng("./res/images/blocks.png"));

    static int culled = 0;
    static int howmany = 0;
    for (int x = -RenderDistance / 2; x < RenderDistance / 2; x++) {
      for (int z = -RenderDistance / 2; z < RenderDistance / 2; z++) {
        const auto boundingBox = Renderer::AABB(
          glm::vec3(x * ChunkSize, MinChunkHeight, z * ChunkSize),
          glm::vec3(x * ChunkSize + ChunkSize, MaxChunkHeight, z * ChunkSize + ChunkSize)
        );

        if (!camera.inView(boundingBox)) {
          culled++;
          continue;
        }

        const auto chunk = Chunk(glm::ivec2(x, z), generateHeightMap(glm::ivec2(x, z)));
        chunks.emplace_back(chunk);
      }
    }

    Logger::logInfo(Logger::Context::Game, "Terrain generated successfully.");
    Logger::logInfo(Logger::Context::Game, "Culled %d chunks.", culled);
  }

  void Terrain::render() const {
    shader.use();
    shader.updateProjectionMatrix(camera.getProjectionMatrix());
    shader.updateViewMatrix(camera.getViewMatrix());
    for (const auto & chunk : chunks) {
      shader.updateModelMatrix(chunk.getModelMatrix());
      chunk.render();
    }
  }

  const Chunk &Terrain::getChunk(const glm::ivec2 &position) {
    return chunks[getChunkIndex(position)];
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
    return (position.x / ChunkSize + position.y / ChunkSize) * ChunkSize;
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
