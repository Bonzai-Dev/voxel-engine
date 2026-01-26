#include <thread>
#include <iostream>
#include <core/logger.hpp>
#include "../camera.hpp"
#include "world.hpp"

#include <map>

using namespace Game::Config;

namespace Game {
  World::World(const Camera &camera) : camera(camera) {
    Logger::logInfo(Logger::Context::Game, "Generating terrain with a seed of %d.", seed);
    shader.use();
    shader.updateTexture(Renderer::loadPng("./res/images/blocks.png"));

    for (size_t threadCount = 0; threadCount < 10; threadCount++) {
      std::thread terrainLoader(&World::loadTerrain, this);
      chunkBuilder.push_back(std::move(terrainLoader));
    }
  }

  World::~World() {
    running = false;

    for (auto &thread: chunkBuilder) {
      if (thread.joinable())
        thread.join();
    }
  }

  void World::render() {
    shader.use();
    shader.updateProjectionMatrix(camera.getProjectionMatrix());
    shader.updateViewMatrix(camera.getViewMatrix());

    static constexpr int halfRenderDistance = (RenderDistance / 2) * ChunkSize;
    const glm::ivec3 &cameraPosition = camera.getPosition();
    const glm::ivec2 cameraGridPosition = glm::ivec2(
      (cameraPosition.x / ChunkSize) * ChunkSize,
      (cameraPosition.z / ChunkSize) * ChunkSize
    );

    const int minimumX = cameraGridPosition.x - halfRenderDistance;
    const int maximumX = cameraGridPosition.x + halfRenderDistance;
    const int minimumZ = cameraGridPosition.y - halfRenderDistance;
    const int maximumZ = cameraGridPosition.y + halfRenderDistance;

    for (auto iterator = chunks.cbegin(); iterator != chunks.cend();) {
      const auto &position = iterator->first;
      const auto &chunk = iterator->second;

      const bool outsideRenderDistance = position.x < minimumX || position.x > maximumX || position.y < minimumZ ||
                                         position.y > maximumZ;
      if (outsideRenderDistance) {
        chunk.deleteBuffers();
        chunks.erase(iterator++);
      } else
        ++iterator;
    }

    // TODO: DEFINE A WATER LEVEL
    std::map<float, glm::vec3> sorted;
    for (auto &[position, chunk]: chunks) {
      static constexpr int halfChunkSize = ChunkSize / 2;
      const glm::vec3 waterMeshPosition = glm::vec3(position.x + halfChunkSize, MinChunkHeight + 1.5f,
                                                    position.y + halfChunkSize);
      const glm::vec3 chunkPosition = glm::vec3(position.x, MinChunkHeight + 1, position.y);

      sorted[glm::length(camera.getPosition() - waterMeshPosition)] = chunkPosition;
    }

    for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
      const glm::ivec3 &chunkPosition = it->second;

      if (chunks.contains({chunkPosition.x, chunkPosition.z})) {
        auto &chunk = chunks.at({chunkPosition.x, chunkPosition.z});
        shader.updateModelMatrix(chunk.getModelMatrix());
        chunk.renderBlocks();
        chunk.renderWater();
      }
    }
  }

  Blocks::BlockId World::getBlockId(const glm::ivec3 &position) {
    return evaluateBlockType(position.y, terrainNoise(glm::ivec2(position.x, position.z)));
  }

  Blocks::BlockId World::evaluateBlockType(int y, int noiseHeight) {
    const bool inChunkHeight = y >= MinChunkHeight && y < MaxChunkHeight;
    if (!inChunkHeight)
      return Blocks::BlockId::Air;

    if (y == MinChunkHeight)
      return Blocks::BlockId::Bedrock;

    if (y == MinChunkHeight + 1)
      return Blocks::BlockId::Dirt;

    if (y == noiseHeight)
      return Blocks::BlockId::Grass;

    if (y < noiseHeight) {
      if (y <= noiseHeight - 10)
        return Blocks::BlockId::Stone;

      return Blocks::BlockId::Dirt;
    }

    if (y == MinChunkHeight + 2)
      return Blocks::BlockId::Water;

    return Blocks::BlockId::Air;
  }

  void World::loadTerrain() {
    while (running) {
      std::lock_guard lock(chunkBuilderMutex);
      static constexpr int halfRenderDistance = (RenderDistance / 2) * ChunkSize;
      const glm::ivec3 &cameraPosition = camera.getPosition();
      const glm::ivec2 cameraGridPosition = glm::ivec2(
        (cameraPosition.x / ChunkSize) * ChunkSize,
        (cameraPosition.z / ChunkSize) * ChunkSize
      );

      for (size_t chunkIndex = 0; chunkIndex < RenderDistance * RenderDistance; chunkIndex++) {
        const int x = cameraGridPosition.x + (chunkIndex % RenderDistance) * ChunkSize - halfRenderDistance;
        const int z = cameraGridPosition.y + ((chunkIndex / RenderDistance) % RenderDistance) * ChunkSize -
                      halfRenderDistance;
        const glm::ivec2 position = glm::ivec2(x, z);

        if (!chunks.contains(ChunkPosition(x, z)))
          chunks.emplace(ChunkPosition(x, z), Chunk(position, generateHeightMap(position / ChunkSize), *this));
      }
    }
  }

  std::vector<int> World::generateHeightMap(const glm::ivec2 &position) {
    std::vector<int> heightMap;
    for (size_t blockCount = 0; blockCount < ChunkSize * ChunkSize; blockCount++) {
      const int x = blockCount % ChunkSize + (position.x * ChunkSize);
      const int z = (blockCount / ChunkSize) % ChunkSize + (position.y * ChunkSize);
      const int y = terrainNoise(glm::ivec2(x, z));
      heightMap.push_back(y);
    }
    return heightMap;
  }

  int World::generateSeed() {
    srand(time(nullptr));
    static constexpr long seedRange = static_cast<long>(10e8);
    return rand() % (2 * seedRange + 1) - seedRange;
  }

  int World::terrainNoise(const glm::ivec2 &position) const {
    int noise = NoiseHeightOffset;
    float scale = NoiseBaseScale;
    float amplitude = NoiseBaseAmplitude;
    for (size_t octave = 1; octave < NoiseOctaves + 1; octave++) {
      scale *= 2.0f;
      amplitude *= 0.5f;
      noise += noise2d(position, glm::vec2(scale, scale), amplitude);
    }

    return noise;
  }

  int World::noise2d(const glm::ivec2 &position, const glm::vec2 &scale, float amplitude) const {
    const auto x = static_cast<double>(position.x);
    const auto y = static_cast<double>(position.y);
    return static_cast<int>(ceilf(noiseGenerator.eval(x * scale.x, y * scale.y) * amplitude));
  }
}
