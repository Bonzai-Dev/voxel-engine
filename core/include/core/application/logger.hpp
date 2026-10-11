#pragma once
#include <memory>
#include <spdlog/spdlog.h>

namespace Core::Application {
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

#define LOG_CORE_TRACE(...) ::Core::Application::Logger::getCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...) ::Core::Application::Logger::getCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARNING(...) ::Core::Application::Logger::getCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) ::Core::Application::Logger::getCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...) ::Core::Application::Logger::getCoreLogger()->critical(__VA_ARGS__)
#define LOG_CORE_DEBUG(...) ::Core::Application::Logger::getCoreLogger()->debug(__VA_ARGS__)

#define LOG_GAME_TRACE(...) ::Core::Application::Logger::getGameLogger()->trace(__VA_ARGS__)
#define LOG_GAME_INFO(...) ::Core::Application::Logger::getGameLogger()->info(__VA_ARGS__)
#define LOG_GAME_WARNING(...)::Core::Application::Logger::getGameLogger()->warn(__VA_ARGS__)
#define LOG_GAME_ERROR(...) ::Core::Application::Logger::getGameLogger()->error(__VA_ARGS__)
#define LOG_GAME_CRITICAL(...) ::Core::Application::Logger::getGameLogger()->critical(__VA_ARGS__)
#define LOG_GAME_DEBUG(...) ::Core::Application::Logger::getGameLogger()->debug(__VA_ARGS__)
