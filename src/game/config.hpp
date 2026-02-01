#pragma once
#include <glm/glm.hpp>

namespace Game {
  class Config {
    public:
      Config();

      ~Config() = default;

      static const glm::vec3 &getAmbientLightColor() { return ambientLightColor; }

      static float getCameraSensitivity() { return cameraSensitivity; }

      static float getCameraSpeed() { return cameraSpeed; }

      static bool randomizeSeed() { return randomize; }

      static int getMaxChunkHeight() { return maxChunkHeight; }

      static int getMinChunkHeight() { return minChunkHeight; }

      static size_t getChunkSize() { return chunkSize; }

      static size_t getChunkHeight() { return chunkHeight; }

      static size_t getTotalChunkBlocks() { return totalChunkBlocks; }

      static int getSeaLevel() { return seaLevel; }

      static int getSandLevel() { return sandLevel; }

      static int getRenderDistance() { return renderDistance; }

      static int getNoiseHeightOffset() { return noiseHeightOffset; }

      static int getNoiseOctaves() { return noiseOctaves; }

      static float getNoiseBaseScale() { return noiseBaseScale; }

      static float getNoiseBaseAmplitude() { return noiseBaseAmplitude; }

      static const glm::vec3 &getSunDirection() { return sunDirection; }

      static const glm::vec3 &getSunColor() { return sunColor; }

      static float getSunBrightness() { return sunBrightness; }

    private:
      static inline glm::vec3 sunDirection;
      static inline glm::vec3 ambientLightColor;
      static inline glm::vec3 sunColor;
      static inline float sunBrightness;

      static inline float cameraSensitivity;
      static inline float cameraSpeed;

      // Whether to randomize the world seed on each launch
      static inline bool randomize;

      // Cannot build above MaxChunkHeight and below MinChunkHeight
      static inline int maxChunkHeight;
      static inline int minChunkHeight;
      static inline size_t chunkSize;
      static inline size_t chunkHeight;
      static inline size_t totalChunkBlocks;
      static inline int seaLevel;
      static inline int sandLevel;

      // How many chunks to render in front of the camera
      static inline int renderDistance;

      static inline int noiseHeightOffset;
      static inline int noiseOctaves;
      static inline float noiseBaseScale;
      static inline float noiseBaseAmplitude;
  };
}
