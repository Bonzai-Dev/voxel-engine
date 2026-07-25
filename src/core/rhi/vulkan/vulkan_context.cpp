#include <core/application/logger.hpp>
#include <core/application/application.hpp>
#include <SDL3/SDL_vulkan.h>
#include "vulkan_rendering_device.hpp"
#include "vulkan_context.hpp"
#include "volk.h"
#include "vk_mem_alloc.h"

namespace Core::Graphics {
  static VkBool32 vulkanDebugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData
  ) {
    switch (messageSeverity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_CORE_TRACE("{}", callbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_CORE_INFO("Vulkan {}", callbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_CORE_WARNING("Vulkan {}", callbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_CORE_ERROR("Vulkan {}", callbackData->pMessage);
        break;

      default:
        break;
    }

    return VK_FALSE;
  }

  VulkanContext::VulkanContext(const char *appName) {
    if (volkInitialize() != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to load Vulkan. Vulkan drivers may be missing on your system.");
      return;
    }

    uint32_t extensionCount = 0;
    VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
    supportedExtensions.resize(extensionCount);
    VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data()));

    uint32_t sdlExtensionCount = 0;
    char const *const*sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
    for (std::uint32_t extensionIndex = 0; extensionIndex < sdlExtensionCount; extensionIndex++) {
      const char *sdlExtension = sdlExtensions[extensionIndex];
      auto findExtension = [&](const VkExtensionProperties &vulkanExtension)-> bool {
        return strcmp(vulkanExtension.extensionName, sdlExtension) == 0;
      };

      if (std::find_if(supportedExtensions.begin(), supportedExtensions.end(), findExtension) == supportedExtensions.end()) {
        LOG_CORE_CRITICAL("Vulkan instance extension \"{}\" is required but not available", sdlExtension);
      }

      addExtension(sdlExtension);
    }

    if constexpr (Application::debugEnabled) {
      addExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      addExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

      for (auto &extension : supportedExtensions)
        LOG_CORE_DEBUG("Found instance extension \"{}\"", extension.extensionName);
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = appName;
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = enabledExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    // Getting instance layers
    const char *validationLayer = "VK_LAYER_KHRONOS_validation";
    uint32_t instanceLayerPropertyCount;
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr);
    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerPropertyCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, instanceLayerProperties.data());
    for (size_t layerIndex = 0; layerIndex < instanceLayerPropertyCount; layerIndex++) {
      const std::string layerName = instanceLayerProperties[layerIndex].layerName;
      if (Application::debugEnabled && layerName == validationLayer) {
        validationLayersEnabled = true;
        instanceLayers.push_back(validationLayer);
      }
      LOG_CORE_TRACE("Found instance layer \"{}\"", layerName);
    }

    if constexpr (Application::debugEnabled) {
      if (std::ranges::find(instanceLayers, validationLayer) != instanceLayers.end())
        LOG_CORE_INFO("Validation layer \"{}\" has been found", validationLayer);
      else
        LOG_CORE_WARNING(
        "Validation layer VK_LAYER_KHRONOS_validation for Vulkan has been requested while in debug mode but is not available"
      );
    }
    instanceCreateInfo.enabledLayerCount = instanceLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();

    VULKAN_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
    volkLoadInstanceOnly(instance);

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
    if (validationLayersEnabled) {
      debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debugMessengerCreateInfo.pNext = nullptr;
      debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debugMessengerCreateInfo.pfnUserCallback = &vulkanDebugUtilsMessengerCallback;
      debugMessengerCreateInfo.pUserData = nullptr;

      instanceCreateInfo.pNext = &debugMessengerCreateInfo;
      VULKAN_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, nullptr, &debugMessenger));
    }

    // Getting GPUs
    std::uint32_t deviceCount = 0;
    VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

    for (VkPhysicalDevice physicalDevice : devices) {
      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

      VkPhysicalDeviceFeatures deviceFeatures;
      vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

      VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
      vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

      RefCountedPtr<VulkanPhysicalDevice> device = RefCountedPtr<VulkanPhysicalDevice>::create(
        physicalDevice,
        deviceFeatures,
        deviceMemoryProperties,
        deviceProperties
      );
      physicalDevices.push_back(device);
      LOG_CORE_DEBUG("Found possible rendering device: {}", device->deviceProperties.deviceName);
    }

    for (RefCountedPtr<VulkanPhysicalDevice> device : physicalDevices) {
      if (device->deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && !selectedDevice) {
        selectedDevice = RefCountedPtr<VulkanDevice>::create(device);
        LOG_CORE_INFO("Selected {} as rendering device", selectedDevice->getPhysicalDevice()->deviceProperties.deviceName);
      }
    }

    // Setting VMA
    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
    vulkanFunctions.vkFreeMemory = vkFreeMemory;
    vulkanFunctions.vkMapMemory = vkMapMemory;
    vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
    vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
    vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
    vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
    vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
    vulkanFunctions.vkCreateImage = vkCreateImage;
    vulkanFunctions.vkDestroyImage = vkDestroyImage;
    vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.device = selectedDevice->logicalDevice;
    allocatorCreateInfo.physicalDevice = selectedDevice->getPhysicalDevice()->physicalDevice;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator);
  }

  VulkanContext::~VulkanContext() {
    for (auto &[windowId, swapChain] : swapChains) {
      swapChain.destroy();
    }

    vmaDestroyAllocator(vmaAllocator);

    selectedDevice->destroy();
    if (debugMessenger)
      vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroyInstance(instance, nullptr);
  }

  void VulkanContext::addExtension(const char *extension) {
    auto extensionSupported = [&](const VkExtensionProperties &vulkanExtension)-> bool {
      return strcmp(vulkanExtension.extensionName, extension) == 0;
    };

    if (std::find_if(supportedExtensions.begin(), supportedExtensions.end(), extensionSupported) == supportedExtensions.
      end()) {
      LOG_CORE_ERROR("Extension \"{}\" is not supported", extension);
      return;
    }

    if (std::find(enabledExtensions.begin(), enabledExtensions.end(), extension) != enabledExtensions.end()) {
      LOG_CORE_ERROR("Extension \"{}\" is already enabled", extension);
      return;
    }

    enabledExtensions.push_back(extension);
  }

  void VulkanContext::createSwapChain(
    SDL_Window *const window,
    std::uint32_t windowId,
    std::uint32_t width,
    std::uint32_t height
  ) const {
    swapChains.emplace(windowId, VulkanSwapChain(instance, selectedDevice, window, width, height));
  }
}
