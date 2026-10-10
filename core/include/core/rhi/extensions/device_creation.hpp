// © 2021 NVIDIA Corporation

// Goal: device creation

#pragma once

#define NRI_DEVICE_CREATION_H 1

namespace Core::RHI {
  enum class Message: uint8_t {
    INFO,
    WARNING,
    ERROR // "wingdi.h" must not be included after
  };

  // Callbacks must be thread safe
  struct AllocationCallbacks {
    void* (*Allocate)(void* userArg, size_t size, size_t alignment);
    void* (*Reallocate)(void* userArg, void* memory, size_t size, size_t alignment);
    void (*Free)(void* userArg, void* memory);
    void* userArg;
    bool disable3rdPartyAllocationCallbacks; // to use "AllocationCallbacks" only for NRI needs
  };

  struct CallbackInterface {
    void (*MessageCallback)(Message messageType, const char* file, uint32_t line, const char* message, void* userArg);
    RHI_OPTIONAL void (*AbortExecution)(void* userArg); // break on "Message::ERROR" if provided
    RHI_OPTIONAL void* userArg;
  };

  // Use largest offset for the resource type planned to be used as an unbounded array
  struct VKBindingOffsets {
    uint32_t sRegister; // samplers
    uint32_t tRegister; // shader resources, including acceleration structures (SRVs)
    uint32_t bRegister; // constant buffers
    uint32_t uRegister; // storage shader resources (UAVs)
  };

  struct VKExtensions {
    const char* const* instanceExtensions;
    uint32_t instanceExtensionNum;
    const char* const* deviceExtensions;
    uint32_t deviceExtensionNum;
  };

  // A collection of queues of the same type
  struct QueueFamilyDesc {
    RHI_OPTIONAL const float* queuePriorities;   // [-1; 1]: low < 0, normal = 0, high > 0 ("queueNum" entries expected)
    uint32_t queueNum;
    QueueType queueType;
  };

  struct DeviceCreationDesc {
    GraphicsBackend graphicsAPI;
    RHI_OPTIONAL Robustness robustness;
    RHI_OPTIONAL const AdapterDesc *adapterDesc;
    RHI_OPTIONAL CallbackInterface callbackInterface;
    RHI_OPTIONAL AllocationCallbacks allocationCallbacks;

    // One "GRAPHICS" queue is created by default
    RHI_OPTIONAL const QueueFamilyDesc *queueFamilies;
    RHI_OPTIONAL uint32_t queueFamilyNum;        // put "GRAPHICS" queue at the beginning of the list

    // D3D specific
    RHI_OPTIONAL uint32_t d3dShaderExtRegister;  // vendor specific shader extensions (default is "NRI_SHADER_EXT_REGISTER", space is always "0")
    RHI_OPTIONAL uint32_t d3dZeroBufferSize;     // no "memset" functionality in D3D, "CmdZeroBuffer" implemented via a bunch of copies (4 Mb by default)

    // Vulkan specific
    VKBindingOffsets vkBindingOffsets;
    RHI_OPTIONAL VKExtensions vkExtensions; // to enable

    // Switches (disabled by default)
    bool enableNRIValidation;                   // embedded validation layer, checks for NRI specifics
    bool enableGraphicsAPIValidation;           // GAPI-provided validation layer
    bool enableD3D11CommandBufferEmulation;     // enable? but why? (auto-enabled if deferred contexts are not supported)
    bool enableD3D12RayTracingValidation;       // slow but useful, can only be enabled if envvar "NV_ALLOW_RAYTRACING_VALIDATION" is set to "1"
    bool enableMemoryZeroInitialization;        // page-clears are fast, but memory is not cleared by default in VK

    // Switches (enabled by default)
    bool disableVKRayTracing;                   // to save CPU memory in some implementations
    bool disableD3D12EnhancedBarriers;          // even if AgilitySDK is in use, some apps still use legacy barriers. It can be important for integrations
  };

  // if "adapterDescs == NULL", then "adapterDescNum" is set to the number of adapters
  // else "adapterDescNum" must be set to number of elements in "adapterDescs"
  Result nriEnumerateAdapters(AdapterDesc *AdapterDescs, uint32_t &adapterDescNum);

  Result nriCreateDevice(const DeviceCreationDesc &DeviceCreationDesc, RHI_OUT Device *&device);
  void nriDestroyDevice(Device *Device);

  // It's global state for D3D, not needed for VK because validation is tied to the logical device
  void nriReportLiveObjects();
}
