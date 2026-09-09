#include <csignal>
#include <cstdio>
#include "vulkan/vulkan_backend.hpp"
#include "rhi.hpp"

namespace {
  using namespace Core::RHI;

  constexpr std::array g_messageTypes = {
    "INFO", // INFO,
    "WARNING", // WARNING,
    "ERROR", // ERROR
  };

  static void MessageCallback(Core::RHI::Message messageType, const char *file, uint32_t line, const char *message,
                              void *) {
    const char *messageTypeName = g_messageTypes[(size_t)messageType];

    char buf[256];
    __gnu_cxx::snprintf(buf, sizeof(buf), "%s (%s:%u) - %s\n", messageTypeName, file, line, message);

    fprintf(stderr, "%s", buf);
#ifdef _WIN32
    OutputDebugStringA(buf);
#endif
  }

  static void AbortExecution(void *) {
#ifdef _WIN32
    DebugBreak();
#else
    std::raise(SIGTRAP);
#endif
  }

  static void *AlignedMalloc(void *, size_t size, size_t alignment) {
    uint8_t *memory = (uint8_t*)malloc(size + sizeof(uint8_t*) + alignment - 1);
    if (!memory)
      return nullptr;

    uint8_t *alignedMemory = Align(memory + sizeof(uint8_t*), alignment);
    uint8_t **memoryHeader = (uint8_t**)alignedMemory - 1;
    *memoryHeader = memory;

    return alignedMemory;
  }

  static void *AlignedRealloc(void *userArg, void *memory, size_t size, size_t alignment) {
    if (!memory)
      return AlignedMalloc(userArg, size, alignment);

    uint8_t **memoryHeader = (uint8_t**)memory - 1;
    uint8_t *oldMemory = *memoryHeader;

    uint8_t *newMemory = (uint8_t*)realloc(oldMemory, size + sizeof(uint8_t*) + alignment - 1);
    if (!newMemory)
      return nullptr;

    if (newMemory == oldMemory)
      return memory;

    uint8_t *alignedMemory = Align(newMemory + sizeof(uint8_t*), alignment);
    memoryHeader = (uint8_t**)alignedMemory - 1;
    *memoryHeader = newMemory;

    return alignedMemory;
  }

  static void AlignedFree(void *, void *memory) {
    if (!memory)
      return;

    uint8_t **memoryHeader = (uint8_t**)memory - 1;
    uint8_t *oldMemory = *memoryHeader;
    free(oldMemory);
  }

  void getVulkanDevices(PhysicalDeviceInfo *physicalDeviceInfos, uint32_t &physicalDevicesCount) {
    VkApplicationInfo applicationInfo = {};
    applicationInfo.apiVersion = VK_API_VERSION_1_2; // New features not needed here as we are only querying device info

    VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    VkInstance instance = VK_NULL_HANDLE;
    DeviceUID uidNeeded = {};
    VkResult vkResult = VK_SUCCESS;
    uint32_t deviceGroupCount = 0;
    uint32_t maxQueueFamilyCount = 1;
    VkPhysicalDeviceGroupProperties *deviceGroupProperties = nullptr;
    VkQueueFamilyProperties2 *familyProps2 = nullptr;

#ifdef ENGINE_PLATFORM_APPLE
    std::array<const char*, 2> instanceExtensions = {
      "VK_KHR_get_physical_device_properties2", VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    };

    instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    // Get loader
    if (volkInitialize() != VK_SUCCESS)
      goto CLEANUP;

    // Loading Vulkan functions manually here as loading volk would require extra steps
    // Get the entry point
    if (!vkGetInstanceProcAddr)
      goto CLEANUP;

    // Create instance
    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
    if (!vkCreateInstance)
      goto CLEANUP;

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
      goto CLEANUP;

    // Get needed functions
    vkDestroyInstance = (PFN_vkDestroyInstance)vkGetInstanceProcAddr(instance, "vkDestroyInstance");
    vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroups");
    vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2");
    vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties");
    vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
    if (!vkDestroyInstance || !vkEnumeratePhysicalDeviceGroups || !vkGetPhysicalDeviceProperties2)
      goto CLEANUP;

    if (vkEnumeratePhysicalDeviceGroups(instance, &deviceGroupCount, nullptr) != VK_SUCCESS)
      goto CLEANUP;

    // Query device groups
    deviceGroupProperties = static_cast<VkPhysicalDeviceGroupProperties*>(
      alloca(sizeof(VkPhysicalDeviceGroupProperties) * deviceGroupCount)
    );
    for (uint32_t i = 0; i < deviceGroupCount; i++) {
      deviceGroupProperties[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
      deviceGroupProperties[i].pNext = nullptr;
    }
    vkEnumeratePhysicalDeviceGroups(instance, &deviceGroupCount, deviceGroupProperties);

    // Max queue families
    if (vkGetPhysicalDeviceQueueFamilyProperties2) {
      for (uint32_t i = 0; i < deviceGroupCount; i++) {
        VkPhysicalDevice physicalDevice = deviceGroupProperties[i].physicalDevices[0];

        uint32_t familyNum = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &familyNum, nullptr);

        maxQueueFamilyCount = std::max(maxQueueFamilyCount, familyNum);
      }
    }

    familyProps2 = static_cast<VkQueueFamilyProperties2*>(
      alloca(sizeof(VkQueueFamilyProperties2) * maxQueueFamilyCount));
    for (uint32_t i = 0; i < maxQueueFamilyCount; i++)
      familyProps2[i] = {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2};

    // Query device groups properties
    for (uint32_t i = 0; i < deviceGroupCount && physicalDevicesCount < maxPhysicalDevicesCount; i++) {
      VkPhysicalDevice physicalDevice = deviceGroupProperties[i].physicalDevices[0];

      VkPhysicalDeviceProperties2 deviceProps2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};

      VkPhysicalDeviceIDProperties deviceIDProperties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};
      deviceProps2.pNext = &deviceIDProperties;

      vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProps2);

      // Logic: append unique or wait for "precreated"
      DeviceUID uid = constructDeviceUID(
        deviceIDProperties.deviceLUID, deviceIDProperties.deviceUUID, deviceIDProperties.deviceLUIDValid
      );

      uint32_t n = 0;
      for (; n < physicalDevicesCount; n++) {
        if (compareDeviceUID(physicalDeviceInfos[n].uid, uid))
          break;
      }

      PhysicalDeviceInfo &physicalDeviceInfo = physicalDeviceInfos[n];

      // Update GAPI support
      physicalDeviceInfo.supportedGraphicsBackends |= GraphicsBackend::Vulkan;

      // Logic: advance or skip
      if (n != physicalDevicesCount)
        continue;

      physicalDevicesCount++;

      // Architecture
      const VkPhysicalDeviceProperties &deviceProps = deviceProps2.properties;

      if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        physicalDeviceInfo.deviceType = DeviceType::Discrete;
      else if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        physicalDeviceInfo.deviceType = DeviceType::Integrated;
      else if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
        physicalDeviceInfo.deviceType = DeviceType::Software;
      else if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
        physicalDeviceInfo.deviceType = DeviceType::Virtual;

      // Memory size
      if (vkGetPhysicalDeviceMemoryProperties) {
        VkPhysicalDeviceMemoryProperties memoryProperties = {};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceMemoryProperties.html
        for (uint32_t j = 0; j < memoryProperties.memoryHeapCount; j++) {
          // From spec: In UMA systems ... implementation must advertise the heap as device-local
          if ((memoryProperties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0 && deviceProps.deviceType
            != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            physicalDeviceInfo.videoMemorySize += memoryProperties.memoryHeaps[j].size;
          else
            physicalDeviceInfo.sharedSystemMemorySize += memoryProperties.memoryHeaps[j].size;
        }
      }

      // Queues
      if (vkGetPhysicalDeviceQueueFamilyProperties2) {
        uint32_t familyNum = maxQueueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &familyNum, familyProps2);

        std::array<uint32_t, static_cast<size_t>(QueueType::Count)> scores = {};
        for (uint32_t j = 0; j < familyNum; j++) {
          const VkQueueFamilyProperties &familyProps = familyProps2[j].queueFamilyProperties;

          QueueFamilyProperties props = {};
          props.queueCount = familyProps.queueCount;
          props.graphics = familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT;
          props.compute = familyProps.queueFlags & VK_QUEUE_COMPUTE_BIT;
          props.copy = familyProps.queueFlags & VK_QUEUE_TRANSFER_BIT;
          props.sparse = familyProps.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
          props.videoDecode = familyProps.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR;
          props.videoEncode = familyProps.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
          props.protect = familyProps.queueFlags & VK_QUEUE_PROTECTED_BIT;
          props.opticalFlow = familyProps.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV;

          QueueType queueType = selectSuitableQueueType(props, scores);
          if (queueType != QueueType::Count)
            physicalDeviceInfo.queueCount[static_cast<size_t>(queueType)] = familyProps.queueCount;
        }
      }

      // Other fields
      physicalDeviceInfo.uid = uid;
      physicalDeviceInfo.deviceId = deviceProps.deviceID;
      physicalDeviceInfo.driverVersion = deviceProps.driverVersion;
      physicalDeviceInfo.vendor = getVendorFromID(deviceProps.vendorID);

      strncpy(physicalDeviceInfo.name, deviceProps.deviceName, sizeof(physicalDeviceInfo.name));
    }

  CLEANUP:
    if (vkDestroyInstance && instance) {
      vkDestroyInstance(instance, nullptr);
    }
  }

  int sortAdapters(const void *pa, const void *pb) {
    constexpr uint64_t SHIFT = 60ull;
    static_assert(static_cast<uint64_t>(DeviceType::Count) <= 1ull << (64ull - SHIFT), "Adjust SHIFT");

    const PhysicalDeviceInfo *a = static_cast<const PhysicalDeviceInfo*>(pa);
    uint64_t sa = a->videoMemorySize + a->sharedSystemMemorySize;
    sa |= static_cast<uint64_t>(a->deviceType) << SHIFT;

    const PhysicalDeviceInfo *b = static_cast<const PhysicalDeviceInfo*>(pb);
    uint64_t sb = b->videoMemorySize + b->sharedSystemMemorySize;
    sb |= static_cast<uint64_t>(b->deviceType) << SHIFT;

    if (sa > sb)
      return -1;

    if (sa < sb)
      return 1;

    return 0;
  }
}

namespace Core::RHI {
  Result getPhysicalDevices(PhysicalDeviceInfo *physicalDeviceInfos, uint32_t &physicalDeviceCount) {
    std::array<PhysicalDeviceInfo, maxPhysicalDevicesCount> scopePhysicalDeviceInfos{};
    uint32_t scopePhysicalDeviceCount = 0;

    getVulkanDevices(scopePhysicalDeviceInfos.data(), scopePhysicalDeviceCount);

    // Sort by video memory size and arhitecture (DISCRETE first)
    qsort(
      scopePhysicalDeviceInfos.data(),
      scopePhysicalDeviceCount,
      sizeof(scopePhysicalDeviceInfos[0]),
      sortAdapters
    );

    // Copy to output
    if (physicalDeviceInfos) {
      physicalDeviceCount = std::min(physicalDeviceCount, scopePhysicalDeviceCount);
      for (uint32_t i = 0; i < physicalDeviceCount; i++) {
        physicalDeviceInfos[i] = scopePhysicalDeviceInfos[i];
      }
    }
    else
      physicalDeviceCount = scopePhysicalDeviceCount;

    return physicalDeviceCount == 0 ? Result::Unsupported : Result::Success;
  }

  Result createDevice(DeviceCreateInfo createInfo, Device*& device) {
    Result result = Result::Unsupported;
    Device *deviceImpl = nullptr;

    createInfo.callbackInterface.MessageCallback = MessageCallback;
    createInfo.callbackInterface.AbortExecution = AbortExecution;

    createInfo.allocationCallbacks.allocate = AlignedMalloc;
    createInfo.allocationCallbacks.reallocate = AlignedRealloc;
    createInfo.allocationCallbacks.free = AlignedFree;

    // TODO: put this in device abstraction
    uint32_t physicalDeviceCount = maxPhysicalDevicesCount;
    std::array<PhysicalDeviceInfo, maxPhysicalDevicesCount> physicalDevices = {};
    getPhysicalDevices(physicalDevices.data(), physicalDeviceCount);
    PhysicalDeviceInfo selectedDevice = physicalDevices[0];
    createInfo.physicalDeviceInfo = &selectedDevice;

    // TODO: add macros checking which OS its on to select the best suitable backend
    if (selectedDevice.supportedGraphicsBackends & GraphicsBackend::Vulkan) {
      result = createVulkanDevice(createInfo, deviceImpl);
    }

    device = deviceImpl;

    return result;
  }
}
