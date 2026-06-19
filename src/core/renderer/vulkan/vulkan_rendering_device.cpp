#include <core/renderer/rendering_device.hpp>
#include <SDL3/SDL_vulkan.h>
#include <core/application/application.hpp>
#include "volk.h"
#include "vulkan_rendering_device.hpp"

namespace Core::Graphics {
  VkBool32 debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData
  ) {
    switch (messageSeverity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_CORE_TRACE("Vulkan {}.", callbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_CORE_INFO("Vulkan {}.", callbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_CORE_WARNING("Vulkan {}.", callbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_CORE_ERROR("Vulkan {}.", callbackData->pMessage);
        break;

      default:
        break;
    }

    return VK_FALSE;
  }

  VulkanRenderingDevice::VulkanRenderingDevice(const char *appName, const DisplayInfo &displayInfo) : RenderingDevice(
    appName, displayInfo) {
    if (volkInitialize() != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to load Vulkan. Vulkan drivers may be missing on your system.");
      return;
    }

    uint32_t extensionCount = 0;
    char const *const*sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    extensions.assign(sdlExtensions, sdlExtensions + extensionCount);

    if (Application::debugEnabled) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    if (Application::debugEnabled) {
      for (auto &extension: extensions)
        LOG_CORE_DEBUG("Found extension \"{}\"", extension);
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = appName;
    applicationInfo.apiVersion = VK_API_VERSION_1_3; // NVRHI only supports 1.3

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = extensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

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
      debugMessengerCreateInfo.pfnUserCallback = &debugCallback;
      debugMessengerCreateInfo.pUserData = nullptr;

      instanceCreateInfo.pNext = &debugMessengerCreateInfo;
    }

    VULKAN_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
    volkLoadInstanceOnly(instance);

    VULKAN_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, nullptr, &debugMessenger));
  }

  void VulkanRenderingDevice::render() {
  }

  VulkanRenderingDevice::~VulkanRenderingDevice() {
    if (debugMessenger)
      vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }
}
