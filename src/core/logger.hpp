#pragma once
#include <string>
#include <array>
#include <cstdint>
#include <iomanip>

// Styling based of the ANSI escape codes
// https://jakob-bagterp.github.io/colorist-for-python/ansi-escape-codes/
namespace Logger {
  constexpr std::int16_t bufferSize = 256;

  enum class Context {
    Core,
    Game,
    Renderer
  };

  enum class Level {
    Info,
    Warning,
    Error
  };

  enum class TextBrightness {
    Standard = 3,
    Bright = 9,
  };

  enum class TextColor {
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
  };

  struct LogStyle {
    const char *level;
    TextColor color;
    TextBrightness brightness;
  };

  constexpr std::array style = {
    LogStyle{"INFO", TextColor::Green, TextBrightness::Bright},
    LogStyle{"WARNING", TextColor::Yellow, TextBrightness::Bright},
    LogStyle{"ERROR", TextColor::Red, TextBrightness::Bright},
  };

  std::string reset();

  std::string colorCode(TextColor color, TextBrightness brightness = TextBrightness::Standard);

  void vlog (Level logLevel, Context context, const char *format, va_list args);

  void log(Level logLevel, Context context, const char *format, ...);

  void logInfo(Context context, const char *format, ...);

  void logWarning(Context context, const char *format, ...);

  void logError(Context context, const char *format, ...);
}
