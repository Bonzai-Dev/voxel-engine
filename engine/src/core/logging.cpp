#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>
#include <engine/core/logging.hpp>

namespace {
  using namespace Engine::Core;

  constexpr quill::LogLevel toQuillLevel(LogLevel level) {
    switch (level) {
      case LogLevel::Trace:
        return quill::LogLevel::TraceL3;
      case LogLevel::Debug:
        return quill::LogLevel::Debug;
      case LogLevel::Info:
        return quill::LogLevel::Info;
      case LogLevel::Warn:
        return quill::LogLevel::Warning;
      case LogLevel::Error:
        return quill::LogLevel::Error;
      case LogLevel::Critical:
        return quill::LogLevel::Critical;
      default:
        return quill::LogLevel::Info;
    }
  }
}

namespace Engine::Core {
  Logger::Logger(const char *name): name(name) {
    static bool initialized = false;
    static const char *formatPattern = "%(time) [%(log_level)] [%(logger)] %(message)";
    static const char *timestampPattern = "%Y-%m-%d %H:%M:%S.%Qms";

    if (!initialized) {
      quill::Backend::start();

#ifdef ENGINE_DEBUG
      logLevel = LogLevel::Trace;
#endif
      initialized = true;
    }

    quill::PatternFormatterOptions options;
    options.format_pattern = formatPattern;
    options.timestamp_pattern = timestampPattern;

    logger = quill::Frontend::create_logger(
      name,
      quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console"),
      options
    );

    logger->set_log_level(toQuillLevel(logLevel));
  }
}
