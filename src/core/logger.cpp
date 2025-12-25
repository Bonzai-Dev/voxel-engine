#include <iomanip>
#include <cstdarg>
#include <iostream>
#include <SDL3/SDL_log.h>
#include "logger.hpp"

namespace Logger {
  std::string reset() {
    return "\033[0m";
  }

  std::string colorCode(TextColor color, TextBrightness brightness) {
    std::stringstream result;
    const auto textColor = static_cast<int>(color);
    const auto textBrightness = static_cast<int>(brightness);
    result << "\033[" << textBrightness << textColor << "m";
    return result.str();
  }

  void vlog(Level logLevel, Context context, const char *format, va_list args) {
    const auto time = std::time(nullptr);
    std::stringstream logTitle;
    const LogStyle logStyle = style[static_cast<int>(logLevel)];

    logTitle << colorCode(logStyle.color, logStyle.brightness);
    logTitle << std::put_time(std::localtime(&time), "[%H:%M:%S]");
    logTitle << " [" << logStyle.level << "] ";

    switch (context) {
      case Context::CORE:
        logTitle << "[CORE] ";
        break;
      case Context::RENDERER:
        logTitle << "[RENDERER] ";
        break;
    }
    logTitle << colorCode(logStyle.color);

    char message[bufferSize];
    vsnprintf(message, bufferSize, format, args);

    std::string_view messageView = message;
    if (messageView.back() == '\n')
      messageView = messageView.substr(0, messageView.size() - 1);
    std::cout << logTitle.view() << messageView << reset() << "\n";
  }

  void log(const Level logLevel, const Context context, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vlog(logLevel, context, format, args);
    va_end(args);
  }

  void logInfo(Context context, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vlog(Level::INFO, context, format, args);
    va_end(args);
  }

  void logWarning(Context context, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vlog(Level::WARNING, context, format, args);
    va_end(args);
  }

  void logError(Context context, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vlog(Level::ERROR, context, format, args);
    va_end(args);
  }
}
