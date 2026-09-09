#include "vulkan_backend.hpp"

namespace Core::RHI {
  Result createVulkanDevice(const DeviceCreateInfo& createInfo, Device *&device) {
    VulkanDevice *impl = RHI::allocate<RHI::VulkanDevice>(
      createInfo.allocationCallbacks,
      createInfo.callbackInterface,
      createInfo.allocationCallbacks
    );
    Result result = impl->create(createInfo);

    if (result != Result::Success) {
      destroy(createInfo.allocationCallbacks, impl);
      device = nullptr;
    }
    else
      device = (Device*)impl;

    return result;
  }
}