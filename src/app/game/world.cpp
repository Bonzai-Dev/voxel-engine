#include <random>
#include <core/logger.hpp>
#include "world.hpp"

namespace Game {
  World::World() {
    seed = generateSeed();
    Logger::logInfo(Logger::Context::Game, "Generating terrain with a seed of %d", seed);
  }

  int World::generateSeed() {
    srand(time(nullptr));
    static constexpr auto seedRange = static_cast<int>(10e5);
    return rand() % (2 * seedRange + 1) - seedRange;
  }

  int World::noise2d(const glm::ivec2 &position, const glm::ivec2 &scale, int amplitude) const {
    return static_cast<int>(floor(noiseGenerator.eval(position.x * scale.x, position.y * scale.y) * amplitude));
  }
}