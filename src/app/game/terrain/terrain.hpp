#pragma once
#include <glm/vec3.hpp>
#include <core/open_simplex2s.hpp>
#include <glm/vec2.hpp>
#include "../camera.hpp"
#include "../shaders/default.hpp"
#include "chunk.hpp"

namespace Game {
  inline constexpr int RenderDistance = 2;

  inline constexpr auto Up = glm::ivec3(0, 1, 0);
  inline constexpr auto Down = glm::ivec3(0, -1, 0);
  inline constexpr auto Forward = glm::ivec3(0, 0, 1);
  inline constexpr auto Backward = glm::ivec3(0, 0, -1);
  inline constexpr auto Left = glm::ivec3(-1, 0, 0);
  inline constexpr auto Right = glm::ivec3(1, 0, 0);

  class Terrain {
    public:
      Terrain(const Camera &camera);

      const int &getSeed() const { return seed; }

      void render() const;

    private:
      int noise2d(const glm::ivec2 &position, const glm::ivec2 &scale, int amplitude) const;

      std::vector<int> generateHeightMap(const glm::ivec2 &chunkPosition) const;

      static int generateSeed();

      const Chunk &getChunk(const glm::ivec2 &position);

      size_t getChunkIndex(const glm::ivec2 &position);

      const Blocks::Block &getBlock(const glm::ivec3 &position);

      int seed = generateSeed();
      Core::OpenSimplexNoise::Noise noiseGenerator = Core::OpenSimplexNoise::Noise(getSeed());
      const Camera &camera;
      Shader::Default shader;
      std::vector<Chunk> chunks;
  };
}
