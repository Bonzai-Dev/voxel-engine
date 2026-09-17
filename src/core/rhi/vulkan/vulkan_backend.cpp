#include "vulkan_backend.hpp"

namespace Core::RHI {
  Result createVulkanDevice(const DeviceCreateInfo& createInfo, Device *&device) {
    VulkanDevice *impl = new VulkanDevice(createInfo.callbackInterface);
    Result result = impl->create(createInfo);

    if (result != Result::Success) {
      delete impl;
      device = nullptr;
    }
    else {
      device = (Device*)impl;
    }

    return result;
  }
}