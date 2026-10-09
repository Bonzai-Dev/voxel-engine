// Goal: wrapping native VK objects into NRI objects

#pragma once

#define NRI_WRAPPER_VK_H 1

#include "device_creation.hpp"

typedef void* VKHandle;
typedef int32_t VKEnum;
typedef uint32_t VKFlags;
typedef uint64_t VKNonDispatchableHandle;

namespace Core::RHI {
  struct AccelerationStructure;

  // A collection of queues of the same type
  struct QueueFamilyVKDesc {
    uint32_t queueNum;
    QueueType queueType;
    uint32_t familyIndex;
  };

  struct DeviceCreationVKDesc {
    CallbackInterface callbackInterface;
    AllocationCallbacks allocationCallbacks;
    const char* libraryPath;
    VKBindingOffsets vkBindingOffsets;
    VKExtensions vkExtensions;                     // enabled
    VKHandle vkInstance;
    VKHandle vkDevice;
    VKHandle vkPhysicalDevice;
    const QueueFamilyVKDesc *queueFamilies;
    uint32_t queueFamilyNum;
    uint8_t minorVersion;                               // >= 2

    // Switches (disabled by default)
    bool enableNRIValidation;
    bool enableMemoryZeroInitialization;                // page-clears are fast, but memory is not cleared by default in VK
  };

  struct CommandAllocatorVKDesc {
    VKNonDispatchableHandle vkCommandPool;
    QueueType queueType;
  };

  struct CommandBufferVKDesc {
    VKHandle vkCommandBuffer;
    QueueType queueType;
  };

  struct DescriptorPoolVKDesc {
    VKNonDispatchableHandle vkDescriptorPool;
    uint32_t descriptorSetMaxNum;
  };

  struct BufferVKDesc {
    VKNonDispatchableHandle vkBuffer;
    uint64_t size;
    uint32_t structureStride;               // must be provided if used as a structured or raw buffer
    uint8_t* mappedMemory;                  // must be provided if the underlying memory is mapped
    VKNonDispatchableHandle vkDeviceMemory; // must be provided *only* if the mapped memory exists and *not* HOST_COHERENT
    uint64_t deviceAddress;                 // must be provided for ray tracing
  };

  struct TextureVKDesc {
    VKNonDispatchableHandle vkImage;
    VKEnum vkFormat;
    VKEnum vkImageType;
    VKFlags vkImageUsageFlags;
    Dim_t width;
    Dim_t height;
    Dim_t depth;
    Dim_t mipNum;
    Dim_t layerNum;
    Sample_t sampleNum;
  };

  struct MemoryVKDesc {
    VKNonDispatchableHandle vkDeviceMemory;
    uint64_t offset;
    void* mappedMemory; // at "offset"
    uint64_t size;
    uint32_t memoryTypeIndex;
  };

  struct PipelineVKDesc {
    VKNonDispatchableHandle vkPipeline;
    VKEnum vkPipelineBindPoint;
  };

  struct QueryPoolVKDesc {
    VKNonDispatchableHandle vkQueryPool;
    VKEnum vkQueryType;
  };

  struct FenceVKDesc {
    VKNonDispatchableHandle vkTimelineSemaphore;
  };

  struct AccelerationStructureVKDesc {
    VKNonDispatchableHandle vkAccelerationStructure;
    VKNonDispatchableHandle vkBuffer;
    uint64_t bufferSize;
    uint64_t buildScratchSize;
    uint64_t updateScratchSize;
    AccelerationStructureBits flags;
  };

  // Threadsafe: yes
  struct WrapperVKInterface {
    Result (*createCommandAllocatorVK)        (const Device &device, const CommandAllocatorVKDesc &commandAllocatorVKDesc, CommandAllocator *&commandAllocator);
    Result (*createCommandBufferVK)           (const Device &device, const CommandBufferVKDesc &commandBufferVKDesc, CommandBuffer *&commandBuffer);
    Result (*createDescriptorPoolVK)          (const Device &device, const DescriptorPoolVKDesc &descriptorPoolVKDesc, DescriptorPool *&descriptorPool);
    Result (*createBufferVK)                  (const Device &device, const BufferVKDesc &bufferVKDesc, Buffer *&buffer);
    Result (*createTextureVK)                 (const Device &device, const TextureVKDesc &textureVKDesc, Texture *&texture);
    Result (*createMemoryVK)                  (const Device &device, const MemoryVKDesc &memoryVKDesc, Memory *&memory);
    Result (*createPipelineVK)                (const Device &device, const PipelineVKDesc &pipelineVKDesc, Pipeline *&pipeline);
    Result (*createQueryPoolVK)               (const Device &device, const QueryPoolVKDesc &queryPoolVKDesc, QueryPool *&queryPool);
    Result (*createFenceVK)                   (const Device &device, const FenceVKDesc &fenceVKDesc, Fence *&fence);
    Result (*createAccelerationStructureVK)   (const Device &device, const AccelerationStructureVKDesc &accelerationStructureVKDesc, AccelerationStructure *&accelerationStructure);

    uint32_t    (*getQueueFamilyIndexVK)           (const Queue &queue);
    VKHandle    (*getPhysicalDeviceVK)             (const Device &device);
    VKHandle    (*getInstanceVK)                   (const Device &device);
    void*       (*getInstanceProcAddrVK)           (const Device &device);
    void*       (*getDeviceProcAddrVK)             (const Device &device);
  };

  Result nriCreateDeviceFromVKDevice(const DeviceCreationVKDesc &DeviceInfo, Device *&device);
}
