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

    ambientLightColor = glm::vec3(
      getIndexValue<double>(0, getData<dom::array>("AmbientLightColor", data)),
      getIndexValue<double>(1, getData<dom::array>("AmbientLightColor", data)),
      getIndexValue<double>(2, getData<dom::array>("AmbientLightColor", data))
    );
    sunDirection = glm::vec3(
      getIndexValue<double>(0, getData<dom::array>("SunDirection", data)),
      getIndexValue<double>(1, getData<dom::array>("SunDirection", data)),
      getIndexValue<double>(2, getData<dom::array>("SunDirection", data))
    );

    sunColor = glm::vec3(
      getIndexValue<double>(0, getData<dom::array>("SunColor", data)),
      getIndexValue<double>(1, getData<dom::array>("SunColor", data)),
      getIndexValue<double>(2, getData<dom::array>("SunColor", data))
    );

    snowHeight = static_cast<int>(getData<std::int64_t>("SnowHeight", data));

    fogFar = static_cast<float>(getData<double>("FogFar", data));
    fogNear = static_cast<float>(getData<double>("FogNear", data));
    fogColor = glm::vec3(
      getIndexValue<double>(0, getData<dom::array>("FogColor", data)),
      getIndexValue<double>(1, getData<dom::array>("FogColor", data)),
      getIndexValue<double>(2, getData<dom::array>("FogColor", data))
    );

    Logger::logInfo(Logger::Context::Game, "Loaded configurations.");
  }
}
