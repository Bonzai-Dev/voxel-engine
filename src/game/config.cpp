#include <glm/vec3.hpp>
#include <util/io.hpp>
#include "config.hpp"

using namespace Util::Json;

namespace Game {
  Config::Config() {
    const auto data = parseJson("./res/config.json");
    noiseBaseAmplitude = static_cast<float>(getData<double>("NoiseBaseAmplitude", data));
    noiseBaseScale = static_cast<float>(getData<double>("NoiseBaseScale", data));
    noiseHeightOffset = static_cast<int>(getData<std::int64_t>("NoiseHeightOffset", data));
    noiseOctaves = static_cast<int>(getData<std::int64_t>("NoiseOctaves", data));

    renderDistance = static_cast<int>(getData<std::int64_t>("RenderDistance", data));

    sandLevel = static_cast<int>(getData<std::int64_t>("SandLevel", data));
    seaLevel = static_cast<int>(getData<std::int64_t>("SeaLevel", data));

    chunkSize = getData<std::int64_t>("ChunkSize", data);
    minChunkHeight = static_cast<int>(getData<std::int64_t>("MinChunkHeight", data));
    maxChunkHeight = static_cast<int>(getData<std::int64_t>("MaxChunkHeight", data));
    chunkHeight = static_cast<size_t>(maxChunkHeight - minChunkHeight);
    totalChunkBlocks = chunkSize * chunkSize * chunkHeight;

    randomize = getData<bool>("Randomize", data);

    cameraSpeed = static_cast<float>(getData<double>("CameraSpeed", data));
    cameraSensitivity = static_cast<float>(getData<double>("CameraSensitivity", data));
    sunBrightness = static_cast<float>(getData<double>("SunBrightness", data));

    const auto ambientLightColorData = getData<dom::array>("AmbientLightColor", data);
    ambientLightColor = glm::vec3(
      getIndexValue<double>(0, ambientLightColorData),
      getIndexValue<double>(1, ambientLightColorData),
      getIndexValue<double>(2, ambientLightColorData)
    );
    const auto sunDirectionData = getData<dom::array>("SunDirection", data);
    sunDirection = glm::vec3(
      getIndexValue<double>(0, sunDirectionData),
      getIndexValue<double>(1, sunDirectionData),
      getIndexValue<double>(2, sunDirectionData)
    );

    const auto sunColorData = getData<dom::array>("SunColor", data);
    sunColor = glm::vec3(
      getIndexValue<double>(0, sunColorData),
      getIndexValue<double>(1, sunColorData),
      getIndexValue<double>(2, sunColorData)
    );

    Logger::logInfo(Logger::Context::Game, "Loaded configurations.");
  }
}
