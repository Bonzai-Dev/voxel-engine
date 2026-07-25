// #include <slang-com-ptr.h>
#include "vulkan_rendering_device.hpp"

namespace Core::Graphics {
  VulkanRenderingDevice::VulkanRenderingDevice(Backend backend, const char *appName, const DisplayInfo &displayInfo) :
  backend(backend), appName(appName), displayInfo(displayInfo), vulkanContext(appName) {
    LOG_CORE_INFO("RENDERER INITIALIZED");
    // Slang::ComPtr<slang::IEntryPoint> entryPoint;
  }

  // VulkanRenderingDevice::~VulkanRenderingDevice() {
  //
  // }

}
