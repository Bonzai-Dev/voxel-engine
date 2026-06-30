#pragma once
#include <unordered_set>
#include <string>
#include <core/memory.hpp>
#include "volk.h"

namespace Core::Graphics {
  class VulkanPhysicalDevice: public RefCounted {
    public:
      VulkanPhysicalDevice(
        VkPhysicalDevice physicalDevice,
        const VkPhysicalDeviceFeatures &deviceFeatures,
        const VkPhysicalDeviceMemoryProperties &deviceMemoryProperties,
        const VkPhysicalDeviceProperties &deviceProperties
      );

      ~VulkanPhysicalDevice() = default;

      const std::unordered_set<std::string> &getExtensions() const { return supportedExtensions; }

      bool extensionSupported(const std::string &extension) const;

      VkPhysicalDevice physicalDevice;
      const VkPhysicalDeviceFeatures deviceFeatures;
      const VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
      const VkPhysicalDeviceProperties deviceProperties;

    private:
      std::vector<VkQueueFamilyProperties> queueFamilyProperties;
      std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

      std::unordered_set<std::string> supportedExtensions;

      int32_t graphicsQueueIndex = -1;
      int32_t computeQueueIndex = -1;
      int32_t transferQueueIndex = -1;
      friend class VulkanDevice;
  };

  class VulkanDevice: public RefCounted {
    public:
      VulkanDevice(RefCountedPtr<VulkanPhysicalDevice> physicalDevice);

      ~VulkanDevice() = default;

      void destroy();

    private:
      VkDevice logicalDevice = VK_NULL_HANDLE;
      RefCountedPtr<VulkanPhysicalDevice> physicalDevice;

      VkQueue graphicsQueue = VK_NULL_HANDLE;
      VkQueue computeQueue = VK_NULL_HANDLE;
  };
}