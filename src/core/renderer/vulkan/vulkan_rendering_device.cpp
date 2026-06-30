#include <core/renderer/rendering_device.hpp>
#include "vulkan_rendering_device.hpp"

namespace Core::Graphics {
  VulkanRenderingDevice::VulkanRenderingDevice(const char *appName, const DisplayInfo &displayInfo) :
  RenderingDevice(appName, displayInfo), vulkanContext(appName) {
    LOG_CORE_INFO("RENDERER INITIALIZED");
  }

  void VulkanRenderingDevice::render() {
  }

  VulkanRenderingDevice::~VulkanRenderingDevice() {

  }
}
