#include <thread>
#include <iostream>
#include <core/logger.hpp>
#include "world.hpp"

namespace Game {
  World::World(const Renderer::Camera &camera) : camera(camera) {
    Logger::logInfo(Logger::Context::Game, "Generating terrain with a seed of %d.", seed);
    shader.use();
    shader.updateTexture(Renderer::loadPng("./res/images/blocks.png"));

    for (size_t threadCount = 0; threadCount < 5; threadCount++) {
      std::thread terrainLoader(&World::loadTerrain, this);
      chunkBuilder.push_back(std::move(terrainLoader));
    }
  }

  World::~World() {
    std::cout << "joined\n";

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

    for (auto iterator = chunks.cbegin(); iterator != chunks.cend(); ) {
      const auto &position = iterator->first;
      const auto &chunk = iterator->second;

      if (position.x < minimumX || position.x > maximumX || position.y < minimumZ || position.y > maximumZ) {
        chunk.deleteBuffers();
        chunks.erase(iterator++);
      }
      else
        ++iterator;
    }

    for (auto &[position, chunk]: chunks) {
      shader.updateModelMatrix(chunk.getModelMatrix());
      chunk.render();
    }
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
        // const auto boundingBox = Renderer::AABB(
        //   glm::vec3(x * ChunkSize, MinChunkHeight, z * ChunkSize),
        //   glm::vec3(x * ChunkSize + ChunkSize, MaxChunkHeight, z * ChunkSize + ChunkSize)
        // );
        // if (!camera.inView(boundingBox))
        //   continue;

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
    int noise = 20;
    static const int octaves = 6;
    static const float baseScale = 0.005f;
    static const float baseAmplitude = 7.0f;
    for (size_t octave = 0; octave < octaves; octave++) {
      const float scale = baseScale * octave;
      noise += noise2d(position, glm::vec2(scale, scale), baseAmplitude * octave);
    }

    return noise;
  }

  int World::noise2d(const glm::ivec2 &position, const glm::vec2 &scale, float amplitude) const {
    const auto x = static_cast<double>(position.x);
    const auto y = static_cast<double>(position.y);
    return static_cast<int>(floor(noiseGenerator.eval(x * scale.x, y * scale.y) * amplitude));
  }
}
