#pragma once
#include <vector>
#include "volk.h"
#include "vulkan_device.hpp"
#include "vulkan_swapchain.hpp"
#include "vk_mem_alloc.h"

namespace Core::Graphics {
  class VulkanSwapChain;

  class VulkanContext {
    public:
      VulkanContext(const char *appName);

      ~VulkanContext();

      VkInstance getInstance() const { return instance; }

      RefCountedPtr<VulkanDevice> getDevice() const { return selectedDevice; }

      const std::unordered_map<std::uint32_t, VulkanSwapChain> &getSwapChains() const { return swapChains; }

      void createSwapChain(
        SDL_Window *window, std::uint32_t windowId,
        std::uint32_t width,
        std::uint32_t height
      ) const;

    private:
      void addExtension(const char* extension);

      std::vector<VkExtensionProperties> supportedExtensions;
      std::vector<const char*> enabledExtensions;

      std::vector<const char*> instanceLayers;
      bool validationLayersEnabled = false;

      static inline VkInstance instance = VK_NULL_HANDLE;
      VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

      VmaAllocator vmaAllocator = nullptr;

      std::vector<RefCountedPtr<VulkanPhysicalDevice>> physicalDevices;
      RefCountedPtr<VulkanDevice> selectedDevice;

      mutable std::unordered_map<std::uint32_t, VulkanSwapChain> swapChains;
  };
}
