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

  class Logger {
    public:
      explicit Logger(const char *name);

      inline operator quill::Logger*() const {
        return logger;
      }

      inline quill::Logger *operator->() const {
        return logger;
      }

    private:
      static inline LogLevel logLevel = LogLevel::Info;

      const char *name;
      quill::Logger *logger = nullptr;
  };
}

#define ENGINE_DEFINE_LOG_CATEGORY(category, name) extern inline const ::Engine::Core::Logger category(name)

#define ENGINE_LOG(logger, level, ...)          \
do {                                            \
  switch (level) {                              \
    case LogLevel::Trace:                       \
      LOG_TRACE_L3(logger, __VA_ARGS__);        \
      break;                                    \
    case LogLevel::Debug:                       \
      LOG_DEBUG(logger, __VA_ARGS__);           \
      break;                                    \
    case LogLevel::Info:                        \
      LOG_INFO(logger, __VA_ARGS__);            \
      break;                                    \
    case LogLevel::Warn:                        \
      LOG_WARNING(logger, __VA_ARGS__);         \
      break;                                    \
    case LogLevel::Error:                       \
      LOG_ERROR(logger, __VA_ARGS__);           \
      break;                                    \
    case LogLevel::Critical:                    \
      LOG_CRITICAL(logger, __VA_ARGS__);        \
      break;                                    \
  }                                             \
} while (0)

#define ENGINE_LOG_TRACE(logger, ...) do { LOG_TRACE_L3(logger, __VA_ARGS__); } while (0)
#define ENGINE_LOG_INFO(logger, ...) do { LOG_INFO(logger, __VA_ARGS__); } while (0)
#define ENGINE_LOG_WARNING(logger, ...) do { LOG_WARNING(logger, __VA_ARGS__); } while (0)
#define ENGINE_LOG_ERROR(logger, ...) do { LOG_ERROR(logger, __VA_ARGS__); } while (0)
#define ENGINE_LOG_CRITICAL(logger, ...) do { LOG_CRITICAL(logger, __VA_ARGS__); } while (0)
