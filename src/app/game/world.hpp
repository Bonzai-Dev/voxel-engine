#pragma once
#include <glm/vec3.hpp>
#include <core/open_simplex2s.hpp>
#include <glm/vec2.hpp>
#include "terrain/chunk.hpp"

namespace Game {
  inline constexpr size_t RenderDistance = 2;

  inline constexpr auto Up = glm::ivec3(0, 1, 0);
  inline constexpr auto Down = glm::ivec3(0, -1, 0);
  inline constexpr auto Forward = glm::ivec3(0, 0, 1);
  inline constexpr auto Backward = glm::ivec3(0, 0, -1);
  inline constexpr auto Left = glm::ivec3(-1, 0, 0);
  inline constexpr auto Right = glm::ivec3(1, 0, 0);

  class World {
    public:
      World();

      const int &getSeed() const { return seed; }

    private:
      int noise2d(const glm::ivec2 &position, const glm::ivec2 &scale, int amplitude) const;

      static int generateSeed();

      Core::OpenSimplexNoise::Noise noiseGenerator;
      std::vector<Terrain::Chunk> chunks;
      int seed = 0;
  };
}
