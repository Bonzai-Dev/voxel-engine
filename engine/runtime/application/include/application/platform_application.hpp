#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <core/logging.hpp>
#include <core/shared_pointer.hpp>

namespace Engine::Application {
  struct MonitorInfo {
    std::string name;

    uint32_t monitorId = 0;
    uint32_t nativeWidth = 0;
    uint32_t nativeHeight = 0;
    uint32_t dpi = 0;
  };

  struct DisplayInfo {
    MonitorInfo *primaryDisplay;
    std::vector<MonitorInfo> monitorInfo;
  };

  class PlatformApplication: public RefCounted {
    public:
      explicit PlatformApplication();

      PlatformApplication(const PlatformApplication &other) = delete;

      PlatformApplication &operator=(const PlatformApplication &other) = delete;

      PlatformApplication(PlatformApplication &&other) = delete;

      PlatformApplication &operator=(PlatformApplication &&other) = delete;

      ~PlatformApplication() override;

      void getDisplayInfo(DisplayInfo &info) const;

      virtual void pollInputs() const;

    private:
      mutable bool running = true;
  };
}
