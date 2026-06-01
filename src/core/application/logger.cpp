#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks-inl.h>
#include <core/application/logger.hpp>
#include "application.hpp"

namespace Core {
  Logger::Logger() {
    spdlog::set_pattern("%^[%Y-%m-%d %H:%M:%S] [%l] [%n] %v%$");
    coreLogger = spdlog::stdout_color_mt("CORE");

    spdlog::level::level_enum logLevel = spdlog::level::info;
    if constexpr (Application::debugEnabled)
      logLevel = spdlog::level::trace;

    coreLogger->set_level(logLevel);

    gameLogger = spdlog::stdout_color_mt("GAME");
    gameLogger->set_level(logLevel);
  }
}
