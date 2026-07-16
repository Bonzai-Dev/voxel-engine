#include <vector>
#include "vulkan_rendering_device.hpp"
#include "vulkan_device.hpp"

namespace Core::Graphics {
  VulkanDevice::VulkanDevice(RefCountedPtr<VulkanPhysicalDevice> physicalDevice): physicalDevice(physicalDevice) {
    std::vector<const char*> deviceExtensions;
    if (!physicalDevice->extensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
      LOG_CORE_CRITICAL("GPU does not support \"{}\" extension", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      return;
    }

    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<std::uint32_t>(physicalDevice->queueCreateInfos.size());;
    deviceCreateInfo.pQueueCreateInfos = physicalDevice->queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &physicalDevice->deviceFeatures;

    if (deviceExtensions.size() > 0) {
      deviceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size());
      deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    }

    VULKAN_CHECK(vkCreateDevice(physicalDevice->physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice));
    volkLoadDevice(logicalDevice);

		vkGetDeviceQueue(logicalDevice, physicalDevice->graphicsQueueIndex, 0, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, physicalDevice->computeQueueIndex, 0, &computeQueue);
  }

  void VulkanDevice::destroy() {
    vkDeviceWaitIdle(logicalDevice);
    vkDestroyDevice(logicalDevice, nullptr);
  }

  VulkanPhysicalDevice::VulkanPhysicalDevice(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceFeatures &deviceFeatures,
    const VkPhysicalDeviceMemoryProperties &deviceMemoryProperties,
    const VkPhysicalDeviceProperties &deviceProperties
  ) : physicalDevice(physicalDevice), deviceFeatures(deviceFeatures), deviceMemoryProperties(deviceMemoryProperties),
      deviceProperties(deviceProperties) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    if (extensionCount > 0) {
      std::vector<VkExtensionProperties> extensions(extensionCount);
      if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, &extensions.front()) ==
        VK_SUCCESS) {
        for (const auto &ext: extensions) {
          supportedExtensions.emplace(ext.extensionName);
        }
      }
    }

    // Queue families
    // Desired queues need to be requested upon logical device creation
    // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
    // requests different queue types

    // Get queue family indices for the requested queue family types
    // Note that the indices may overlap depending on the implementation

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    static const float defaultQueuePriority(0.0f);
    int requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
      for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
        VkQueueFamilyProperties &properties = queueFamilyProperties[i];
        if ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) && ((properties.queueFlags &
          VK_QUEUE_GRAPHICS_BIT) == 0)) {
          computeQueueIndex = i;
          break;
        }
      }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
      for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
        VkQueueFamilyProperties &properties = queueFamilyProperties[i];
        if ((properties.queueFlags & VK_QUEUE_TRANSFER_BIT) && ((properties.queueFlags &
          VK_QUEUE_GRAPHICS_BIT) == 0) && ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
          transferQueueIndex = i;
          break;
        }
      }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
      if ((requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) && transferQueueIndex == -1) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
          transferQueueIndex = i;
      }

      if ((requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) && computeQueueIndex == -1) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
          computeQueueIndex = i;
      }

      if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
          graphicsQueueIndex = i;
      }
    }

    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = graphicsQueueIndex;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
    }

    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
      if (computeQueueIndex != graphicsQueueIndex) {
        // If compute family index differs, we need an additional queue create info for the compute queue
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = computeQueueIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
      }
    }

    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
      if ((transferQueueIndex != graphicsQueueIndex) && (transferQueueIndex != computeQueueIndex)) {
        // If compute family index differs, we need an additional queue create info for the compute queue
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = transferQueueIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
      }
    }
  }

  bool VulkanPhysicalDevice::extensionSupported(const std::string &extension) const {
    return supportedExtensions.find(extension) != supportedExtensions.end();
  }

  VkFormat VulkanPhysicalDevice::getDepthFormat() const {
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<VkFormat> depthFormats = {
      VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_D24_UNORM_S8_UINT,
      VK_FORMAT_D16_UNORM_S8_UINT,
      VK_FORMAT_D16_UNORM
    };

    // TODO: Move to VulkanPhysicalDevice
    for (auto& format : depthFormats)
    {
      VkFormatProperties formatProps;
      vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
      // Format must support depth stencil attachment for optimal tiling
      if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        return format;
    }
    return VK_FORMAT_UNDEFINED;

  }
}
