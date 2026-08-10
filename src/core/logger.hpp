#pragma once
#include <memory>
#include <spdlog/spdlog.h>

namespace Core {
  class Application;

  class Logger {
    public:
      Logger();

      static std::shared_ptr<spdlog::logger> getCoreLogger() { return coreLogger; }

      static std::shared_ptr<spdlog::logger> getGameLogger() { return gameLogger; }

    private:
      static inline std::shared_ptr<spdlog::logger> coreLogger;
      static inline std::shared_ptr<spdlog::logger> gameLogger;
  };
}

#define LOG_CORE_TRACE(...) ::Core::Logger::getCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...) ::Core::Logger::getCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARNING(...) ::Core::Logger::getCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) ::Core::Logger::getCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...) ::Core::Logger::getCoreLogger()->critical(__VA_ARGS__)
#define LOG_CORE_FATAL(...) ::Core::Logger::getCoreLogger()->fatal(__VA_ARGS__)
#define LOG_CORE_DEBUG(...) ::Core::Logger::getCoreLogger()->debug(__VA_ARGS__)

#define LOG_GAME_TRACE(...) ::Core::Logger::getGameLogger()->trace(__VA_ARGS__)
#define LOG_GAME_INFO(...) ::Core::Logger::getGameLogger()->info(__VA_ARGS__)
#define LOG_GAME_WARNING(...)::Core::Logger::getGameLogger()->warn(__VA_ARGS__)
#define LOG_GAME_ERROR(...) ::Core::Logger::getGameLogger()->error(__VA_ARGS__)
#define LOG_GAME_CRITICAL(...) ::Core::Logger::getGameLogger()->critical(__VA_ARGS__)
#define LOG_GAME_FATAL(...) ::Core::Logger::getGameLogger()->fatal(__VA_ARGS__)
#define LOG_GAME_DEBUG(...) ::Core::Logger::getGameLogger()->debug(__VA_ARGS__)
