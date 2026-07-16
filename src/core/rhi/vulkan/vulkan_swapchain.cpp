#include <SDL3/SDL_vulkan.h>
#include <core/application/logger.hpp>
#include "vulkan_swapchain.hpp"
#include "vulkan_rendering_device.hpp"
#include "vulkan_context.hpp"
#include "volk.h"

namespace Core::Graphics {
  VulkanSwapChain::VulkanSwapChain(
    const VkInstance &instance,
    RefCountedPtr<VulkanDevice> device,
    SDL_Window *const window,
    std::uint32_t width,
    std::uint32_t height
  ) : window(window), instance(instance), device(device) {
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
      LOG_CORE_ERROR("Failed to create Vulkan window surface");
      return;
    }

    VkPhysicalDevice physicalDevice = device->getPhysicalDevice()->physicalDevice;

    // Get list of supported surface formats
    uint32_t formatCount;
    VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
    if (formatCount == 0) {
      LOG_CORE_ERROR("No supported surface formats found");
      return;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data()));

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
      surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
      surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
    }
    else {
      // iterate over the list of available surface format and
      // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
      bool found_B8G8R8A8_UNORM = false;
      for (auto &&surfaceFormat : surfaceFormats) {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
          this->surfaceFormat.format = surfaceFormat.format;
          this->surfaceFormat.colorSpace = surfaceFormat.colorSpace;
          found_B8G8R8A8_UNORM = true;
          break;
        }
      }

      // in case VK_FORMAT_B8G8R8A8_UNORM is not available
      // select the first available color format
      if (!found_B8G8R8A8_UNORM) {
        surfaceFormat.format = surfaceFormats[0].format;
        surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
      }
    }

    // Creating swap chain
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice,
      surface,
      &surfaceCapabilities
    );

    VkExtent2D swapChainExtents{surfaceCapabilities.currentExtent};
    if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
      swapChainExtents = {.width = width, .height = height};

    VkSwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.minImageCount = surfaceCapabilities.minImageCount;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = {.width = swapChainExtents.width, .height = swapChainExtents.height};
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    VULKAN_CHECK(vkCreateSwapchainKHR(device->logicalDevice, &swapChainCreateInfo, nullptr, &swapChain));

    std::uint32_t imageCount = 0;
    VULKAN_CHECK(vkGetSwapchainImagesKHR(device->logicalDevice, swapChain, &imageCount, nullptr));
    images.resize(imageCount);
    VULKAN_CHECK(vkGetSwapchainImagesKHR(device->logicalDevice, swapChain, &imageCount, images.data()));
    imageViews.resize(imageCount);

    
  }

  void VulkanSwapChain::destroy() {
    vkDestroySwapchainKHR(device->logicalDevice, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
  }
}
