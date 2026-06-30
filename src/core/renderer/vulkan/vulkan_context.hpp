#pragma once
#include <vector>
#include "volk.h"
#include "vulkan_device.hpp"

namespace Core::Graphics {
  class VulkanContext {
    public:
      VulkanContext(const char *appName);

      ~VulkanContext();

      VkInstance getInstance() { return instance; }

    private:
      std::vector<const char*> extensions;
      std::vector <const char*> instanceLayers;
      bool validationLayersEnabled = false;

      static inline VkInstance instance = VK_NULL_HANDLE;
  		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

      std::vector<RefCountedPtr<VulkanPhysicalDevice>> physicalDevices;

      RefCountedPtr<VulkanPhysicalDevice> selectedPhysicalDevice;
      RefCountedPtr<VulkanDevice> device;
  };
}
