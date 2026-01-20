#pragma once
#include <thread>
#include <mutex>
#include <glm/vec3.hpp>
#include <core/open_simplex2s.hpp>
#include <glm/vec2.hpp>
#include <core/renderer/camera/camera.hpp>
#include <app/game/shaders/default.hpp>
#include "chunk.hpp"

namespace Game {
  // How many chunks to render in front of the camera
  inline constexpr int RenderDistance = 10;

  inline constexpr auto Up = glm::ivec3(0, 1, 0);
  inline constexpr auto Down = glm::ivec3(0, -1, 0);
  inline constexpr auto Forward = glm::ivec3(0, 0, 1);
  inline constexpr auto Backward = glm::ivec3(0, 0, -1);
  inline constexpr auto Left = glm::ivec3(-1, 0, 0);
  inline constexpr auto Right = glm::ivec3(1, 0, 0);

  class Terrain {
    public:
      Terrain(const Renderer::Camera &camera);

      ~Terrain();

      const int &getSeed() const { return seed; }

      void render();

    private:
      Core::OpenSimplexNoise::Noise noiseGenerator = Core::OpenSimplexNoise::Noise(getSeed());
      const Renderer::Camera &camera;
      Shader::Default shader;
      std::vector<Chunk> chunks;
      std::vector<std::thread> chunkBuilder;
      std::mutex chunkMutex;

      // Position is the actual position of chunk, not the index
      void loadChunk(const glm::ivec2 &position);

      void loadTerrain();

      void joinThreads();

      int noise2d(const glm::ivec2 &position, const glm::vec2 &scale, float amplitude) const;

      int terrainNoise(const glm::ivec2 &position);

      std::vector<int> generateHeightMap(const glm::ivec2 &position);

      static int generateSeed();

      int seed = generateSeed();
  };
}
