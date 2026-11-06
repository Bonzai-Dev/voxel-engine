#pragma once
#include <memory>
#include <engine/core/logging.hpp>
#include <engine/memory/intrusive_ptr.hpp>

namespace Engine::Application {
  ENGINE_DEFINE_LOG_CATEGORY(appLogger, "application");

  struct MonitorInfo {
    uint32_t monitorId = 0;
    uint32_t nativeWidth = 0;
    uint32_t nativeHeight = 0;
    uint32_t dpi = 0;
    bool primaryDisplay = false;
    std::string name;
  };

  struct DisplayInfo {
    MonitorInfo *primaryDisplay;
    std::vector<MonitorInfo> monitorInfo;
  };

  class PlatformApplication: RefCounted {
    public:
      explicit PlatformApplication();

      PlatformApplication(const PlatformApplication &other) = delete;

      PlatformApplication &operator=(const PlatformApplication &other) = delete;

      PlatformApplication(PlatformApplication &&other) = delete;

      PlatformApplication &operator=(PlatformApplication &&other) = delete;

      virtual unsigned long addRef() {
        return ++referenceCount;
      }

      virtual unsigned long release() {
        unsigned long result = --referenceCount;
        if (result == 0) {
          delete this;
        }
        return result;
      }

      ~PlatformApplication() override;

      void getDisplayInfo(DisplayInfo &info) const;

      virtual void pollInputs() const;

    private:
      // mutable std::unique_ptr<Renderer::Renderer> renderer;

      mutable bool running = true;
  };
}
