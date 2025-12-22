#pragma once

#include <string>
#include <array>
#include <cstdint>
#include <iomanip>

// Styling based of the ANSI escape codes
// https://jakob-bagterp.github.io/colorist-for-python/ansi-escape-codes/

namespace logger {
  constexpr std::int16_t bufferSize = 256;

  enum class Context {
    CORE,
    RENDERER
  };

  enum class Level {
    INFO,
    WARNING,
    ERROR
  };

  enum class TextBrightness {
    STANDARD = 3,
    BRIGHT = 9,
  };

  enum class TextColor {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
  };

  struct LogStyle {
    const char *level;
    TextColor color;
    TextBrightness brightness;
  };

  constexpr std::array style = {
    LogStyle{"INFO", TextColor::GREEN, TextBrightness::BRIGHT},
    LogStyle{"WARNING", TextColor::YELLOW, TextBrightness::BRIGHT},
    LogStyle{"ERROR", TextColor::RED, TextBrightness::BRIGHT},
  };

  std::string reset();

  std::string colorCode(TextColor color, TextBrightness brightness = TextBrightness::STANDARD);

  void log(Level logLevel, Context context, const char *format, ...);

  void logInfo(Context context, const char *format, ...);

  void logWarning(Context context, const char *format, ...);

  void logError(Context context, const char *format, ...);
}
