#pragma once
#include <memory>
#include <format>

namespace spdlog {
  class logger;
}

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
      Logger(const char *name);

      template<typename... Args>
      void log(LogLevel level, std::format_string<Args...> format, Args&&... args) const {
        const std::string formated = std::format(format, std::forward<Args>(args)...);
        output(level, formated);
      }

    private:
      void output(LogLevel level, std::string_view message) const;

      static inline LogLevel logLevel = LogLevel::Info;

      std::shared_ptr<spdlog::logger> logger;
  };
}

#define ENGINE_DEFINE_LOG_CATEGORY(category, name) extern inline const ::Engine::Core::Logger category(name)

#define ENGINE_LOG(category, level, ...) category.log(level, __VA_ARGS__)
#define ENGINE_LOG_TRACE(category, ...) category.log(::Engine::Core::LogLevel::Trace, __VA_ARGS__)
#define ENGINE_LOG_INFO(category, ...) category.log(::Engine::Core::LogLevel::Info, __VA_ARGS__)
#define ENGINE_LOG_WARNING(category, ...) category.log(::Engine::Core::LogLevel::Warn, __VA_ARGS__)
#define ENGINE_LOG_ERROR(category, ...) category.log(::Engine::Core::LogLevel::Error, __VA_ARGS__)
#define ENGINE_LOG_CRITICAL(category, ...) category.log(::Engine::Core::LogLevel::Critical, __VA_ARGS__)
#define ENGINE_LOG_DEBUG(category, ...) category.log(::Engine::Core::LogLevel::Debug, __VA_ARGS__)
