#pragma once
#include <quill/Logger.h>
#include <quill/LogMacros.h>

namespace Engine::Core {
  enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Critical = 5,
  };

  class Logger;

  namespace detail {
    inline quill::Logger *getLogger(const Logger &logger);
  }

  class Logger {
    public:
      explicit Logger(const char *name);

    private:
      friend quill::Logger *detail::getLogger(const Logger &logger);

      static inline LogLevel logLevel = LogLevel::Info;

      const char *name;
      quill::Logger *logger = nullptr;
  };

  namespace detail {
    inline quill::Logger *getLogger(const Logger &logger) {
      return logger.logger;
    }
  }
}

#define ENGINE_DEFINE_LOG_CATEGORY(category, name) \
  inline const ::Engine::Core::Logger category = ::Engine::Core::Logger(name)

#define ENGINE_LOG_TRACE(logger, fmt, ...) do { LOG_TRACE_L3(::Engine::Core::detail::getLogger(logger), fmt, __VA_ARGS__); } while (0)
#define ENGINE_LOG_INFO(logger, fmt, ...) do { LOG_INFO(::Engine::Core::detail::getLogger(logger), fmt, __VA_ARGS__); } while (0)
#define ENGINE_LOG_WARNING(logger, fmt, ...) do { LOG_WARNING(::Engine::Core::detail::getLogger(logger), fmt, __VA_ARGS__); } while (0)
#define ENGINE_LOG_ERROR(logger, fmt, ...) do { LOG_ERROR(::Engine::Core::detail::getLogger(logger), fmt, __VA_ARGS__); } while (0)
#define ENGINE_LOG_CRITICAL(logger, fmt, ...) do { LOG_CRITICAL(::Engine::Core::detail::getLogger(logger), fmt, __VA_ARGS__); } while (0)

#define ENGINE_LOG(logger, level, fmt, ...)             \
  do {                                                  \
    switch (level) {                                    \
      case LogLevel::Trace:                             \
        ENGINE_LOG_TRACE(logger, fmt, __VA_ARGS__);     \
        break;                                          \
      case LogLevel::Debug:                             \
        ENGINE_LOG_DEBUG(logger, fmt, __VA_ARGS__);     \
        break;                                          \
      case LogLevel::Info:                              \
        ENGINE_LOG_INFO(logger, fmt, __VA_ARGS__);      \
        break;                                          \
      case LogLevel::Warn:                              \
        ENGINE_LOG_WARNING(logger, fmt, __VA_ARGS__);   \
        break;                                          \
      case LogLevel::Error:                             \
        ENGINE_LOG_ERROR(logger, fmt, __VA_ARGS__);     \
        break;                                          \
      case LogLevel::Critical:                          \
        ENGINE_LOG_CRITICAL(logger, fmt, __VA_ARGS__);  \
        break;                                          \
    }                                                   \
  } while (0)
