#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks-inl.h>
#include <engine/core/logging.hpp>

namespace Engine::Core {
  Logger::Logger(const char *name) {
    logger = spdlog::stdout_color_mt(name);

    static bool initialized = false;
    if (!initialized) {
      // The log level is mapped exactly to how the spdlog log level is.
      // No need to convert it to the spdlog log level.
#ifdef ENGINE_DEBUG
      logLevel = LogLevel::Trace;
#endif

      spdlog::set_pattern("%^[%Y-%m-%d %H:%M:%S] [%l] [%n] %v%$");
      initialized = true;
    }

    logger->set_level(static_cast<spdlog::level::level_enum>(logLevel));
  }

  void Logger::output(LogLevel level, std::string_view formated) const {
    switch (level) {
      case LogLevel::Trace:
        logger->trace(formated);
        break;
      case LogLevel::Debug:
        logger->debug(formated);
        break;
      case LogLevel::Info:
        logger->info(formated);
        break;
      case LogLevel::Warn:
        logger->warn(formated);
        break;
      case LogLevel::Error:
        logger->error(formated);
        break;
      case LogLevel::Critical:
        logger->critical(formated);
        break;
      default:
        break;
    }
  }
}
