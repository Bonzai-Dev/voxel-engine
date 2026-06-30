#pragma once
#include <core/renderer/window.hpp>

namespace Core::Graphics {
  class VulkanWindow: public Window {
    public:
      VulkanWindow(
        const DisplayInfo &displayInfo,
        const WindowOptions &windowOptions
      );

      ~VulkanWindow() override;

    private:

  };
}