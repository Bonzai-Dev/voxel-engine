#pragma once
#include <unordered_set>
#include <string>
#include <core/memory.hpp>
#include <slang/slang.h>
#include "volk.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace Core::Graphics {
  struct Vertex {
    glm::vec4 pos;
  };

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

      int32_t getGraphicsQueueIndex() const { return graphicsQueueIndex; }

      bool extensionSupported(const std::string &extension) const;

      VkFormat getDepthFormat() const;

      const VkPhysicalDevice physicalDevice =  VK_NULL_HANDLE;
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

      RefCountedPtr<VulkanPhysicalDevice> getPhysicalDevice() const { return physicalDevice; }

      VkQueue getGraphicsQueue() const { return graphicsQueue; }

      VkDevice logicalDevice = VK_NULL_HANDLE;

    private:
      RefCountedPtr<VulkanPhysicalDevice> physicalDevice;

      VkQueue graphicsQueue = VK_NULL_HANDLE;
      VkQueue computeQueue = VK_NULL_HANDLE;
  };
}
