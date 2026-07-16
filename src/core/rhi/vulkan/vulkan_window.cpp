#include <SDL3/SDL_vulkan.h>
#include "vulkan_window.hpp"
#include "vulkan_rendering_device.hpp"
#include "volk.h"

namespace Core::Graphics {
  VulkanWindow::VulkanWindow(
    const VulkanContext &vulkanContext,
    const DisplayInfo &displayInfo,
    const WindowOptions &windowOptions
  ) : Window(displayInfo, windowOptions), context(vulkanContext) {
    vulkanContext.createSwapChain(window, getId(), displayInfo.width, displayInfo.height);
  }
}
