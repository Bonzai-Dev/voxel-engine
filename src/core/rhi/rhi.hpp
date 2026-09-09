#pragma once
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <array>
#include <core/core.hpp>
#include <core/memory/intrusive_ptr.hpp>
#include "stl/allocator.hpp"
#include "stl/creation.hpp"

/*
Overview:
 - Generalized common denominator for VK, D3D12 and D3D11
    - VK spec: https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html
       - Best practices: https://developer.nvidia.com/blog/vulkan-dos-donts/
       - Feature support coverage: https://vulkan.gpuinfo.org/
    - D3D12 spec: https://microsoft.github.io/DirectX-Specs/
       - Feature support coverage: https://d3d12infodb.boolka.dev/
    - D3D11 spec: https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm

Goals:
 - a Graphics API, not RHI!
 - generalization and unification of D3D12 and VK
 - explicitness (providing access to low-level features of modern GAPIs)
 - quality-of-life and high-level extensions (e.g., streaming and upscaling)
 - low overhead
 - cross-platform and platform independence (AMD/INTEL friendly)
 - D3D11 support (as much as possible)

Non-goals:
 - exposing entities not existing in GAPIs
 - high-level (D3D11-like) abstraction
 - hidden management of any kind (except for some high-level extensions where it's desired)
 - automatic barriers (better handled in a higher-level abstraction)

Thread safety:
 - Threadsafe: yes - free-threaded access
 - Threadsafe: no  - external synchronization required, i.e. one thread at a time (additional restrictions can apply)
 - Threadsafe: ?   - unclear status

Implicit:
 - Create*         - thread safe
 - Destroy*        - not thread safe (because of VK)
 - Cmd*            - not thread safe
*/

namespace Core::RHI {
  constexpr uint32_t invalidQueueFamilyIndex = static_cast<uint32_t>(-1);
  constexpr uint32_t maxPhysicalDevicesCount = 32;
  constexpr uint32_t fenceTimeout = 5000u;

/* clang-format off */
  //============================================================================================================================================================================================
#pragma region [ Common ]
  //============================================================================================================================================================================================
  class Resource: public RefCounted {
    public:
      Resource() = default;

      ~Resource() override = default;

      unsigned long addRef() override {
        return ++referenceCount;
      }

      unsigned long release() override {
        unsigned long result = --referenceCount;
        if (result == 0) {
          delete this;
        }
        return result;
      }
  };

  enum class Result: int8_t {
    // All bad, but optionally require an action ("callbackInterface.AbortExecution" is not triggered)
    DeviceLost = -3, // may be returned by "QueueSubmit*", "*WaitIdle", "AcquireNextTexture", "QueuePresent", "WaitForPresent"
    OutOfDate = -2, // VK: swap chain is out of date, can be triggered if "features.resizableSwapChain" is not supported; D3D12: shader cache is stale
    InvalidSDK = -1, // D3D12: some interfaces are missing (potential reasons: unable to load "D3D12Core.dll", version or SDK mismatch, developer mode is not enabled)

    // All good
    Success = 0,

    // All bad, most likely a crash or a validation error will happen next ("callbackInterface.AbortExecution" is triggered)
    Failure = 1,
    InvalidArgument = 2,
    OutOfMemory = 3,
    Unsupported = 4 // if enabled, NRI validation can promote some to "INVALID_ARGUMENT"
  };

  constexpr const char *resultToString(Result result) {
    switch (result) {
      case Result::DeviceLost: return "Device lost";
      case Result::OutOfDate: return "Out of date";
      case Result::InvalidSDK: return "Invalid SDK";
      case Result::Success: return "Success";
      case Result::Failure: return "Failure";
      case Result::InvalidArgument: return "Invalid argument";
      case Result::OutOfMemory: return "Out of memory";
      case Result::Unsupported: return "Unsupported";
      default: return "Unknown result";
    }
  }

  struct DepthStencil {
    float depth;
    uint8_t stencil;
  };

  struct Color32f {
    float x, y, z, w;
  };

  struct Color32ui {
    uint32_t x, y, z, w;
  };

  struct Color32i {
    int32_t x, y, z, w;
  };

  union Color {
    Color32f f;
    Color32ui ui;
    Color32i i;
  };

  union ClearValue {
    DepthStencil depthStencil;
    Color color;
  };
#pragma endregion

  //============================================================================================================================================================================================
#pragma region [ Pipeline stages and barriers ]
//============================================================================================================================================================================================
  // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html
  // https://docs.vulkan.org/samples/latest/samples/performance/pipeline_barriers/README.html

  // A barrier consists of two phases:
  // - before (source scope, 1st synchronization scope):
  //   - "AccessBits" corresponding with any relevant resource usage since the preceding barrier or the start of "QueueSubmit" scope
  //   - "StagesBits" of all preceding GPU work that must be completed before executing the barrier (stages to wait before the barrier)
  //   - "Layout" for textures
  // - after (destination scope, 2nd synchronization scope):
  //   - "AccessBits" corresponding with any relevant resource usage after the barrier completes
  //   - "StagesBits" of all subsequent GPU work that must wait until the barrier execution is finished (stages to halt until the barrier is executed)
  //   - "Layout" for textures
  // If "features.enhancedBarriers" is not supported:
  //   - https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#compatibility-with-legacy-d3d12_resource_states
  //   - "AccessBits::NONE" gets mapped to "COMMON" (aka "GENERAL" access), leading to potential discrepancies with VK

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineStageFlagBits2.html
  // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#d3d12_barrier_sync
  ENGINE_ENUM_BITS(StageBits, uint32_t,
    // Special
    All                             = 0,            // Lazy default for barriers                          Shader stage
    None                            = 0x7FFFFFFF,

    // Graphics                                    // Invoked by "CmdDraw*"
    IndexInput                      = ENGINE_BIT(0),    //    Index buffer consumption
    VertexShader                    = ENGINE_BIT(1),    //    Vertex shader                                   X (required within GRAPHICS bind point)
    TessellationControlShader       = ENGINE_BIT(2),    //    Tessellation control (hull) shader              X
    TessellationEvaluationShader    = ENGINE_BIT(3),    //    Tessellation evaluation (domain) shader         X
    GeometryShader                  = ENGINE_BIT(4),    //    Geometry shader                                 X
    TaskShader                      = ENGINE_BIT(5),    //    Task (amplification) shader                     X
    MeshShader                      = ENGINE_BIT(6),    //    Mesh shader                                     X (or required within GRAPHICS bind point)
    FragmentShader                  = ENGINE_BIT(7),    //    Fragment (pixel) shader                         X
    DepthStencilAttachment          = ENGINE_BIT(8),    //    Depth-stencil R/W operations
    ColorAttachment                 = ENGINE_BIT(9),    //    Color R/W operations
    ShadingRateAttachment           = ENGINE_BIT(10),   //    Shading rate attachment R

    // Compute                                     // Invoked by "CmdDispatch*" (not Rays)
    ComputeShader                   = ENGINE_BIT(11),   //    Compute shader                                  X (required within COMPUTE bind point)

    // Ray tracing                                 // Invoked by "CmdDispatchRays*"
    RayGenShader                    = ENGINE_BIT(12),   //    Ray generation shader                           X (required within RAY_TRACING bind point)
    MissShader                      = ENGINE_BIT(13),   //    Miss shader                                     X
    IntersectionShader              = ENGINE_BIT(14),   //    Intersection shader                             X
    ClosestHitShader                = ENGINE_BIT(15),   //    Closest hit shader                              X
    AnyHitShader                    = ENGINE_BIT(16),   //    Any hit shader                                  X
    CallableShader                  = ENGINE_BIT(17),   //    Callable shader                                 X
    AccelerationStructure           = ENGINE_BIT(18),   // Invoked by "Cmd*AccelerationStructure*" commands
    Micromap                        = ENGINE_BIT(19),   // Invoked by "Cmd*Micromap*" commands

    // Other
    Copy                            = ENGINE_BIT(20),   // Invoked by "CmdCopy*", "CmdUpload*" and "CmdReadback*"
    Resolve                         = ENGINE_BIT(21),   // Invoked by "CmdResolveTexture"
    ClearStorage                    = ENGINE_BIT(22),   // Invoked by "CmdClearStorage"

    // Modifiers
    Indirect                        = ENGINE_BIT(23),   // Invoked by "Indirect" commands (used in addition to other bits)

    // Umbrella stages
    TessellationShaders             = TessellationControlShader | TessellationEvaluationShader,

    MeshShaders                     = TaskShader | MeshShader,

    GraphicsShaders                 = VertexShader | TessellationShaders | GeometryShader | MeshShaders | FragmentShader,

    RayTracingShaders               = RayGenShader | MissShader | IntersectionShader | ClosestHitShader | AnyHitShader | CallableShader,

    AllShaders                      = GraphicsShaders | ComputeShader | RayTracingShaders,

    Graphics                        = IndexInput | GraphicsShaders | DepthStencilAttachment | ColorAttachment | ShadingRateAttachment
  );
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Device description and capabilities ]
//============================================================================================================================================================================================
  ENGINE_ENUM_BITS(GraphicsBackend, uint8_t,
    None    = ENGINE_BIT(0), // Supports everything, does nothing, returns dummy non-NULL objects and ~0-filled descs, available if "NRI_ENABLE_NONE_SUPPORT = ON" in CMake
    D3D11   = ENGINE_BIT(1), // Direct3D 11 (feature set 11.1), available if "NRI_ENABLE_D3D11_SUPPORT = ON" in CMake (https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm)
    D3D12   = ENGINE_BIT(2), // Direct3D 12 (D3D12_SDK_VERSION 4 or 619+), available if "NRI_ENABLE_D3D12_SUPPORT = ON" in CMake (https://microsoft.github.io/DirectX-Specs/)
    Vulkan  = ENGINE_BIT(3) // Vulkan 1.4+, 1.3++ or 1.2+++ (can be used on MacOS via MoltenVK), available if "NRI_ENABLE_VK_SUPPORT = ON" in CMake (https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html)
    // WGPU    = NriBit(4)  // WebGPU via wgpu-native, available if "NRI_ENABLE_WGPU_SUPPORT = ON" in CMake (https://github.com/gfx-rs/wgpu-native)
  );

  // enum class GraphicsBackend: uint8_t {
  //   None    = ENGINE_BIT(0), // Supports everything, does nothing, returns dummy non-NULL objects and ~0-filled descs, available if "NRI_ENABLE_NONE_SUPPORT = ON" in CMake
  //   D3D11   = ENGINE_BIT(1), // Direct3D 11 (feature set 11.1), available if "NRI_ENABLE_D3D11_SUPPORT = ON" in CMake (https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm)
  //   D3D12   = ENGINE_BIT(2), // Direct3D 12 (D3D12_SDK_VERSION 4 or 619+), available if "NRI_ENABLE_D3D12_SUPPORT = ON" in CMake (https://microsoft.github.io/DirectX-Specs/)
  //   Vulkan  = ENGINE_BIT(3), // Vulkan 1.4+, 1.3++ or 1.2+++ (can be used on MacOS via MoltenVK), available if "NRI_ENABLE_VK_SUPPORT = ON" in CMake (https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html)
  //   // WGPU    = NriBit(4)  // WebGPU via wgpu-native, available if "NRI_ENABLE_WGPU_SUPPORT = ON" in CMake (https://github.com/gfx-rs/wgpu-native)
  // };

  // TODO: temporary
  enum class Message {
    INFO,
    WARNING,
    ERROR,
    MAX_NUM
  };

  struct CallbackInterface {
    void (*MessageCallback)(Message messageType, const char *file, uint32_t line, const char *message, void *userArg);
    void (*AbortExecution)(void *userArg); // break on "Message::ERROR" if provided
    void *userArg;
  };

  class Device {
    public:
      Device(const CallbackInterface& callbacks, const AllocationCallbacks& allocationCallbacks);

      virtual ~Device() = default;

      CallbackInterface callbackInterface;
      AllocationCallbacks allocationCallbacks;
      StdAllocator<uint8_t> stdAllocator;
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkQueueFlagBits.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_command_list_type
  enum class QueueType: uint8_t {
    Graphics,
    Compute,
    Copy,
    Count, // Used to represent the queue count. Not a valid queue type
  };

  struct QueueFamilyInfo {
    const float *queuePriorities; // [-1; 1]: low < 0, normal = 0, high > 0 ("queueNum" entries expected)
    std::uint32_t queueCount;
    QueueType queueType;
  };

  struct QueueFamilyProperties {
    uint32_t queueCount;
    bool graphics;
    bool compute;
    bool copy;
    bool sparse;
    bool videoDecode;
    bool videoEncode;
    bool protect;
    bool opticalFlow;
  };

  class Queue: public Resource {
    public:
      Queue() = default;

      ~Queue() override = default;
  };

  ENGINE_FORCE_INLINE QueueType selectSuitableQueueType(
    const QueueFamilyProperties& props,
    std::array<uint32_t, static_cast<size_t>(QueueType::Count)> &scores
  ) {
    { // Prefer as much features as possible
      size_t index = static_cast<size_t>(QueueType::Graphics);
      uint32_t score = ((props.graphics ? 100 : 0) + (props.compute ? 10 : 0) + (props.copy ? 10 : 0) + (props.sparse ? 5 : 0) + (props.videoDecode ? 2 : 0) + (props.videoEncode ? 2 : 0) + (props.protect ? 1 : 0) + (props.opticalFlow ? 1 : 0));

      if (props.graphics && score > scores[index]) {
        scores[index] = score;
        return QueueType::Graphics;
      }
    }

    { // Prefer compute-only
      size_t index = static_cast<size_t>(QueueType::Compute);
      uint32_t score = ((!props.graphics ? 10 : 0) + (props.compute ? 100 : 0) + (!props.copy ? 10 : 0) + (props.sparse ? 5 : 0) + (!props.videoDecode ? 2 : 0) + (!props.videoEncode ? 2 : 0) + (props.protect ? 1 : 0) + (!props.opticalFlow ? 1 : 0));

      if (props.compute && score > scores[index]) {
        scores[index] = score;
        return QueueType::Compute;
      }
    }

    { // Prefer copy-only
      size_t index = static_cast<size_t>(QueueType::Copy);
      uint32_t score = ((!props.graphics ? 10 : 0) + (!props.compute ? 10 : 0) + (props.copy ? 100 * props.queueCount : 0) + (props.sparse ? 5 : 0) + (!props.videoDecode ? 2 : 0) + (!props.videoEncode ? 2 : 0) + (props.protect ? 1 : 0) + (!props.opticalFlow ? 1 : 0));

      if (props.copy && score > scores[index]) {
        scores[index] = score;
        return QueueType::Copy;
      }
    }

    return QueueType::Count;
  }

  struct VulkanExtensions {
    const char *const*instanceExtensions;
    uint32_t instanceExtensionCount;
    const char *const*deviceExtensions;
    uint32_t deviceExtensionCount;
  };

  struct VulkanBindingOffsets {
    uint32_t sRegister; // samplers
    uint32_t tRegister; // shader resources, including acceleration structures (SRVs)
    uint32_t bRegister; // constant buffers
    uint32_t uRegister; // storage shader resources (UAVs)
  };

  // https://docs.vulkan.org/guide/latest/robustness.html
  enum class Robustness: uint8_t {
    Default, // don't care, follow device settings (VK level when used on a device)
    Off, // no overhead, no robust access (out-of-bounds access is not allowed)
    Vulkan, // minimal overhead, partial robust access
    D3D12 // moderate overhead, D3D12-level robust access (requires "VK_EXT_robustness2", soft fallback to VK mode)
  };

  enum class Vendor: uint8_t {
    Unknown,
    Nvidia,
    AMD,
    Intel
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceType.html
  enum class DeviceType: uint8_t {
    Unknown,
    Software, // CPU
    Virtual, // remote desktop?
    Integrated, // UMA
    Discrete, // yes, please!
    Count
  };

  struct DeviceUID {
    uint64_t low;
    uint64_t high;
  };

  struct PhysicalDeviceInfo {
    char name[256]{};
    DeviceUID uid{}; // "LUID" (preferred) if "uid.high = 0", or "UUID" otherwise
    uint64_t videoMemorySize{};
    uint64_t sharedSystemMemorySize{};
    uint32_t deviceId{};
    uint32_t driverVersion{}; // GAPI and OS dependent
    std::array<uint32_t, static_cast<uint32_t>(QueueType::Count)> queueCount;
    Vendor vendor = Vendor::Unknown;
    DeviceType deviceType = DeviceType::Unknown;
    GraphicsBackend supportedGraphicsBackends = GraphicsBackend::None;
  };

  struct DeviceCreateInfo {
    // GraphicsBackend graphicsBackend = GraphicsBackend::None;
    Robustness robustness{};
    PhysicalDeviceInfo *physicalDeviceInfo{};
    CallbackInterface callbackInterface{};
    AllocationCallbacks allocationCallbacks{};

    // One "GRAPHICS" queue is created by default
    const QueueFamilyInfo *queueFamilies{};
    uint32_t queueFamilyCount{}; // put "GRAPHICS" queue at the beginning of the list

    // D3D specific
    uint32_t d3dShaderExtRegister{};
    // vendor specific shader extensions (default is "NRI_SHADER_EXT_REGISTER", space is always "0")
    uint32_t d3dZeroBufferSize{};
    // no "memset" functionality in D3D, "CmdZeroBuffer" implemented via a bunch of copies (4 Mb by default)

    // Vulkan specific
    VulkanBindingOffsets vulkanBindingOffsets{};
    VulkanExtensions vulkanExtensions{}; // Extensions to enable

    // Switches (disabled by default)
    bool enableNRIValidation{}; // embedded validation layer, checks for NRI specifics
    bool enableGraphicsAPIValidation{}; // GAPI-provided validation layer
    bool enableD3D11CommandBufferEmulation{}; // enable? but why? (auto-enabled if deferred contexts are not supported)
    bool enableD3D12RayTracingValidation{};
    // slow but useful, can only be enabled if envvar "NV_ALLOW_RAYTRACING_VALIDATION" is set to "1"
    bool enableMemoryZeroInitialization{}; // page-clears are fast, but memory is not cleared by default in VK

    // Switches (enabled by default)
    bool disableVKRayTracing{}; // to save CPU memory in some implementations
    bool disableD3D12EnhancedBarriers{};
    // even if AgilitySDK is in use, some apps still use legacy barriers. It can be important for integrations
  };

  // Feature support coverage: https://vulkan.gpuinfo.org/ and https://d3d12infodb.boolka.dev/
  struct DeviceInfo {
    // Common
    PhysicalDeviceInfo physicalDeviceInfo; // "queueCount" reflects available number of queues per "QueueType"
    GraphicsBackend graphicsBackend = GraphicsBackend::None;
    uint16_t nriVersion{};
    uint16_t shaderModel{}; // see "NriShaderModel"

    // Viewport
    struct {
      uint32_t maxNum;
      int32_t boundsMin;
      int32_t boundsMax;
    } viewport{};

    // Dimensions
    struct {
      uint32_t typedBufferMaxDim;
      uint16_t attachmentMaxDim;
      uint16_t attachmentLayerMaxNum;
      uint16_t texture1DMaxDim;
      uint16_t texture2DMaxDim;
      uint16_t texture3DMaxDim;
      uint16_t textureLayerMaxNum;
    } dimensions{};

    // Precision bits
    struct {
      uint32_t viewportBits;
      uint32_t subPixelBits;
      uint32_t subTexelBits;
      uint32_t mipmapBits;
    } precision{};

    // Memory
    struct {
      uint64_t deviceUploadHeapSize; // ReBAR
      uint64_t bufferMaxSize;
      uint64_t allocationMaxSize;
      uint32_t allocationMaxNum;
      uint32_t samplerAllocationMaxNum;
      uint32_t constantBufferMaxRange;
      uint32_t storageBufferMaxRange;
      uint32_t bufferTextureGranularity;
      // specifies a page-like granularity at which linear and non-linear resources must be placed in adjacent memory locations to avoid aliasing
      uint32_t alignmentDefault;
      // (INTERNAL) worst-case alignment for a memory allocation respecting all possible placed resources, excluding multisample textures
      uint32_t alignmentMultisample;
      // (INTERNAL) worst-case alignment for a memory allocation respecting all possible placed resources, including multisample textures
    } memory{};

    // Memory alignment requirements
    struct {
      uint32_t uploadBufferTextureRow;
      uint32_t uploadBufferTextureSlice;
      uint32_t bufferShaderResourceOffset;
      uint32_t constantBufferOffset;
      uint32_t scratchBufferOffset;
      uint32_t shaderBindingTable;
      uint32_t accelerationStructureOffset;
      uint32_t micromapOffset;
    } memoryAlignment{};

    // Pipeline layout (see "FitPipelineLayoutSettingsIntoDeviceLimits")
    // D3D12 only: "rootConstantSize" + "descriptorSetNum" * 4 + "rootDescriptorNum" * 8 + "reservedSize" <= 256, where
    // "reservedSize" is 8 bytes for "ENABLE_DRAW_PARAMETERS_EMULATION" and 4 bytes for "ENABLE_DRAW_INDEX_EMULATION"
    struct {
      uint32_t descriptorSetMaxNum;
      uint32_t rootConstantMaxSize;
      uint32_t rootDescriptorMaxNum;
    } pipelineLayout{};

    // Descriptor set
    struct {
      uint32_t samplerMaxNum;
      uint32_t constantBufferMaxNum;
      uint32_t storageBufferMaxNum;
      uint32_t textureMaxNum;
      uint32_t storageTextureMaxNum;

      struct {
        uint32_t samplerMaxNum;
        uint32_t constantBufferMaxNum;
        uint32_t storageBufferMaxNum;
        uint32_t textureMaxNum;
        uint32_t storageTextureMaxNum;
      } updateAfterSet;
    } descriptorSet{};

    // Shader stages
    struct {
      // Per stage resources
      uint32_t descriptorSamplerMaxNum;
      uint32_t descriptorConstantBufferMaxNum;
      uint32_t descriptorStorageBufferMaxNum;
      uint32_t descriptorTextureMaxNum;
      uint32_t descriptorStorageTextureMaxNum;
      uint32_t resourceMaxNum;

      struct {
        uint32_t descriptorSamplerMaxNum;
        uint32_t descriptorConstantBufferMaxNum;
        uint32_t descriptorStorageBufferMaxNum;
        uint32_t descriptorTextureMaxNum;
        uint32_t descriptorStorageTextureMaxNum;
        uint32_t resourceMaxNum;
      } updateAfterSet;

      // Vertex
      struct {
        uint32_t attributeMaxNum;
        uint32_t streamMaxNum;
        uint32_t outputComponentMaxNum;
      } vertex;

      // Tessellation control
      struct {
        float generationMaxLevel;
        uint32_t patchPointMaxNum;
        uint32_t perVertexInputComponentMaxNum;
        uint32_t perVertexOutputComponentMaxNum;
        uint32_t perPatchOutputComponentMaxNum;
        uint32_t totalOutputComponentMaxNum;
      } tesselationControl;

      // Tessellation evaluation
      struct {
        uint32_t inputComponentMaxNum;
        uint32_t outputComponentMaxNum;
      } tesselationEvaluation;

      // Geometry
      struct {
        uint32_t invocationMaxNum;
        uint32_t inputComponentMaxNum;
        uint32_t outputComponentMaxNum;
        uint32_t outputVertexMaxNum;
        uint32_t totalOutputComponentMaxNum;
      } geometry;

      // Fragment
      struct {
        uint32_t inputComponentMaxNum;
        uint32_t attachmentMaxNum;
        uint32_t dualSourceAttachmentMaxNum;
      } fragment;

      // Compute
      //  - a "dispatch" consists of "work groups" (aka "thread groups")
      //  - a "work group" consists of "waves" (aka "subgroups" or "warps")
      //  - a "wave" consists of "lanes", which can can be active, inactive or a helper:
      //    - active: the "lane" is performing its computations
      //    - inactive: the "lane" is part of the "wave" but is currently masked out
      //    - helper: these "lanes" are executed to provide auxiliary information (like derivatives) for active threads in the same 2x2 quad
      //  - "invocation" (or "thread") is a single shader instance
      //  - "lane" specifically refers to the position of a "thread" within a hardware "wave"
      //  - the concept of "wave/lane" execution applies to all shader stages
      struct {
        uint32_t dispatchMaxDim[3];
        uint32_t workGroupInvocationMaxNum;
        uint32_t workGroupMaxDim[3];
        uint32_t sharedMemoryMaxSize;
      } compute;

      // Task
      struct {
        uint32_t dispatchWorkGroupMaxNum;
        uint32_t dispatchMaxDim[3];
        uint32_t workGroupInvocationMaxNum;
        uint32_t workGroupMaxDim[3];
        uint32_t sharedMemoryMaxSize;
        uint32_t payloadMaxSize;
      } task;

      // Mesh
      struct {
        uint32_t dispatchWorkGroupMaxNum;
        uint32_t dispatchMaxDim[3];
        uint32_t workGroupInvocationMaxNum;
        uint32_t workGroupMaxDim[3];
        uint32_t sharedMemoryMaxSize;
        uint32_t outputVerticesMaxNum;
        uint32_t outputPrimitiveMaxNum;
        uint32_t outputComponentMaxNum;
      } mesh;

      // Ray tracing
      struct {
        uint32_t shaderGroupIdentifierSize;
        uint32_t shaderBindingTableMaxStride;
        uint32_t recursionMaxDepth;
      } rayTracing;
    } shaderStage{};

    // Acceleration structure
    struct {
      uint64_t primitiveMaxNum; // per BLAS
      uint64_t geometryMaxNum; // per BLAS
      uint64_t instanceMaxNum; // per TLAS
      uint32_t micromapSubdivisionMaxLevel;
    } accelerationStructure{};

    // Wave (subgroup)
    // https://github.com/microsoft/directxshadercompiler/wiki/wave-intrinsics
    // https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Derivatives.html
    struct {
      uint32_t laneMinNum;
      uint32_t laneMaxNum;
      StageBits waveOpsStages; // SM 6.0+ (see "shaderFeatures.waveX")
      StageBits quadOpsStages; // SM 6.0+ (see "shaderFeatures.waveQuad")
      StageBits derivativeOpsStages;
      // SM 6.6+ (https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Derivatives.html#derivative-functions)
    } wave{};

    // Other
    struct {
      uint64_t timestampFrequencyHz;
      uint32_t drawIndirectMaxNum;
      float samplerLodBiasMax;
      float samplerAnisotropyMax;
      int8_t texelGatherOffsetMin;
      int8_t texelOffsetMin;
      uint8_t texelOffsetMax;
      uint8_t texelGatherOffsetMax;
      uint8_t clipDistanceMaxNum;
      uint8_t cullDistanceMaxNum;
      uint8_t combinedClipAndCullDistanceMaxNum;
      uint8_t viewMaxNum; // multiview is supported if > 1
      uint8_t shadingRateAttachmentTileSize; // square size
    } other{};

    // Tiers (0 - unsupported)
    struct {
      // https://microsoft.github.io/DirectX-Specs/d3d/ConservativeRasterization.html#tiered-support
      // 1 - 1/2 pixel uncertainty region and does not support post-snap degenerates
      // 2 - reduces the maximum uncertainty region to 1/256 and requires post-snap degenerates not be culled
      // 3 - maintains a maximum 1/256 uncertainty region and adds support for inner input coverage, aka "SV_InnerCoverage"
      uint8_t conservativeRaster;

      // https://microsoft.github.io/DirectX-Specs/d3d/ProgrammableSamplePositions.html#hardware-tiers
      // 1 - a single sample pattern can be specified to repeat for every pixel ("locationNum / sampleNum" ratio must be 1 in "CmdSetSampleLocations"),
      //     1x and 16x sample counts do not support programmable locations
      // 2 - four separate sample patterns can be specified for each pixel in a 2x2 grid ("locationNum / sampleNum" ratio can be 1 or 4 in "CmdSetSampleLocations"),
      //     all sample counts support programmable positions
      uint8_t sampleLocations;

      // https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#checkfeaturesupport-structures
      // 1 - DXR 1.0: full raytracing functionality, except features below
      // 2 - DXR 1.1: adds - ray query, "CmdDispatchRaysIndirect", "GeometryIndex()" intrinsic, additional ray flags & vertex formats
      // 3 - DXR 1.2: adds - micromap, shader execution reordering
      uint8_t rayTracing;

      // https://microsoft.github.io/DirectX-Specs/d3d/VariableRateShading.html#feature-tiering
      // 1 - shading rate can be specified only per draw
      // 2 - adds: per primitive shading rate, per "shadingRateAttachmentTileSize" shading rate, combiners, "SV_ShadingRate" support
      uint8_t shadingRate;

      // https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#limitations-on-static-samplers
      // 0 - ALL descriptors in range must be valid by the time the command list executes
      // 1 - only "CONSTANT_BUFFER" and "STORAGE" descriptors in range must be valid
      // 2 - only referenced descriptors must be valid
      uint8_t resourceBinding;

      // 1 - unbound arrays with dynamic indexing
      // 2 - D3D12 dynamic resources: https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_DynamicResources.html
      uint8_t bindless;

      // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_heap_tier
      // 1 - a "Memory" can support resources from all 3 categories: buffers, attachments, all other textures
      uint8_t memory;
    } tiers{};

    // Features
    struct {
      // Swap chain
      bool swapChain; // NRISwapChain
      bool presentFromCompute; // see "SwapChainDesc::queue"
      bool waitableSwapChain; // see "SwapChainDesc::waitable"
      bool resizableSwapChain; // swap chain can be resized without triggering an "OUT_OF_DATE" error

      // Multi view
      bool flexibleMultiview; // see "Multiview::FLEXIBLE"
      bool layerBasedMultiview; // see "Multiview::LAYRED_BASED"
      bool viewportBasedMultiview; // see "Multiview::VIEWPORT_BASED"

      // Texture compression
      bool textureCompressionBC; // all "BC" texture formats are supported
      bool textureCompressionETC2; // all "ETC2" texture formats are supported
      bool textureCompressionASTC; // all "ASTC" texture formats are supported

      // Shader bytecode
      bool shaderBytecodeDXBC; // DXBC can be passed to "ShaderDesc::bytecode"
      bool shaderBytecodeDXIL; // DXIL can be passed to "ShaderDesc::bytecode"
      bool shaderBytecodeSPIRV; // SPIRV can be passed to "ShaderDesc::bytecode", WGPU expects Vulkan 1.2 environment
      bool shaderBytecodeWGSL; // WGSL can be passed to "ShaderDesc::bytecode"

      // Queries
      bool occlusion; // see "QueryType::OCCLUSION"
      bool timestamp; // see "QueryType::TIMESTAMP"
      bool timestampCopyQueue; // see "QueryType::TIMESTAMP_COPY_QUEUE"
      bool calibratedTimestamps; // see "GetCalibratedTimestamps"

      // Shading rate
      bool additionalShadingRates; // see "ShadingRate"
      bool sumShadingRateCombiner; // see "ShadingRateCombiner::SUM"

      // Resolve
      bool regionResolve; // see "CmdResolveTexture"
      bool resolveOpMinMax; // see "ResolveOp"

      // Pipeline cache
      bool pipelineCache; // "PipelineCache" support (NOP fallback if unsupported, except on error)
      bool pipelineCacheControl;
      // "FAIL_ON_CACHE_MISS" enforces "FAILURE", useful for platforms that prohibit runtime PSO compilation (e.g., Xbox GDK)

      // Other
      bool getMemoryDesc2; // "GetXxxMemoryDesc2" support (VK: requires "maintenance4", D3D: supported)
      bool enhancedBarriers; // VK: supported, D3D12: requires "AgilitySDK", D3D11: unsupported
      bool tessellationShader; // Tessellation control and evaluation shader stages
      bool geometryShader; // Geometry shader stage
      bool meshShader; // NRIMeshShader
      bool lowLatency; // NRILowLatency
      bool componentSwizzle; // see "ComponentSwizzle" (unsupported only in D3D11)
      bool independentFrontAndBackStencilReferenceAndMasks; // see "StencilAttachmentDesc::back"
      bool filterOpMinMax; // see "FilterOp"
      bool logicOp; // see "LogicOp"
      bool depthBoundsTest; // see "DepthAttachmentDesc::boundsTest"
      bool drawIndirectCount; // see "countBuffer" and "countBufferOffset"
      bool lineSmoothing; // see "RasterizationDesc::lineSmoothing"
      bool meshShaderPipelineStats; // see "PipelineStatisticsDesc"
      bool dynamicDepthBias; // see "CmdSetDepthBias"
      bool viewportOriginBottomLeft; // see "Viewport"
      bool pipelineStatistics; // see "QueryType::PIPELINE_STATISTICS"
      bool rootConstantsOffset; // see "SetRootConstantsDesc" (unsupported only in D3D11)
      bool nonConstantBufferRootDescriptorOffset; // see "SetRootDescriptorDesc" (unsupported only in D3D11)
      bool mutableDescriptorType; // see "DescriptorType::MUTABLE"
      bool extendedDynamicState;
      // VK: allows to use "VertexBufferDesc::stride" (dynamic) instead of "VertexStreamDesc::stride" (static). Widely supported
      bool unifiedTextureLayouts;
      // VK: allows to use "GENERAL" everywhere: https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_unified_image_layouts.html
    } features{};

    // Shader features
    // https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst
    struct {
      // Native types (I32 and F32 are always supported)
      // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-scalar
      bool nativeI8; // "(u)int8_t"
      bool nativeI16; // "(u)int16_t"
      bool nativeF16; // "float16_t"
      bool nativeI64; // "(u)int64_t"
      bool nativeF64; // "double"

      // Atomics on native types (I32 atomics are always supported, for others it can be partial support of SMEM, texture or buffer atomics)
      // https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-cs-atomic-functions
      // https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Int64_and_Float_Atomics.html
      bool atomicsI16; // "(u)int16_t" atomics
      bool atomicsF16; // "float16_t" atomics
      bool atomicsF32; // "float" atomics
      bool atomicsI64; // "(u)int64_t" atomics
      bool atomicsF64; // "double" atomics

      // Storage without format
      // https://learn.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads#using-unorm-and-snorm-typed-uav-loads-from-hlsl
      bool storageReadWithoutFormat; // NRI_FORMAT("unknown") is allowed for storage reads
      bool storageWriteWithoutFormat; // NRI_FORMAT("unknown") is allowed for storage writes

      // Wave intrinsics
      // https://github.com/microsoft/directxshadercompiler/wiki/wave-intrinsics
      bool waveQuery; // WaveIsFirstLane, WaveGetLaneCount, WaveGetLaneIndex
      bool waveVote; // WaveActiveAllTrue, WaveActiveAnyTrue, WaveActiveAllEqual
      bool waveShuffle; // WaveReadLaneFirst, WaveReadLaneAt
      bool waveArithmetic;
      // WaveActiveSum, WaveActiveProduct, WaveActiveMin, WaveActiveMax, WavePrefixProduct, WavePrefixSum
      bool waveReduction;
      // WaveActiveCountBits, WaveActiveBitAnd, WaveActiveBitOr, WaveActiveBitXor, WavePrefixCountBits
      bool waveQuad; // QuadReadLaneAt, QuadReadAcrossX, QuadReadAcrossY, QuadReadAcrossDiagonal

      // Other
      bool viewportIndex; // SV_ViewportArrayIndex, always can be used in geometry shaders
      bool layerIndex; // SV_RenderTargetArrayIndex, always can be used in geometry shaders
      bool unnormalizedCoordinates;
      // https://microsoft.github.io/DirectX-Specs/d3d/VulkanOn12.html#non-normalized-texture-sampling-coordinates
      bool clock; // https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#readclock
      bool rasterizedOrderedView;
      // https://microsoft.github.io/DirectX-Specs/d3d/RasterOrderViews.html (aka fragment shader interlock)
      bool barycentric; // https://github.com/microsoft/DirectXShaderCompiler/wiki/SV_Barycentrics
      bool rayTracingPositionFetch;
      // https://docs.vulkan.org/features/latest/features/proposals/VK_KHR_ray_tracing_position_fetch.html
      bool integerDotProduct; // https://github.com/microsoft/DirectXShaderCompiler/wiki/Shader-Model-6.4
      bool inputAttachments;
      // https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#subpass-inputs

      bool drawParameters;
      // GAPI-independent "NRI_BASE_VERTEX", "NRI_BASE_INSTANCE", "NRI_VERTEX_ID_OFFSET" and "NRI_INSTANCE_ID_OFFSET" (see "NRI.hlsl" for expected usage)
      bool drawIndex; // GAPI-independent "NRI_DRAW_ID" (see "NRI.hlsl" for expected usage)
    } shaderFeatures{};
  };

  // Memory requirements for a resource (buffer or texture)
  struct MemoryInfo {
    uint64_t size;
    uint32_t alignment;
    uint32_t type; // Contains some encoded implementation specific details
    bool mustBeDedicated; // must be put into a dedicated "Memory" object, containing only 1 object with offset = 0
  };

  // NRI tries to ease your life and avoid using "queue ownership transfers" (see "TextureBarrierDesc").
  // In most of cases "SharingMode" can be ignored. Where is it needed?
  // - VK: use "EXCLUSIVE" for attachments participating into multi-queue activities to preserve DCC (Delta Color Compression) on some HW
  // - D3D12: use "SIMULTANEOUS" to concurrently use a texture as a "SHADER_RESOURCE" (or "SHADER_RESOURCE_STORAGE") and as a "COPY_DESTINATION" for non overlapping texture regions
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkSharingMode.html
  enum class SharingMode: uint8_t {
    Concurrent,     // VK: lazy default to avoid dealing with "queue ownership transfers", auto-optimized to "EXCLUSIVE" if all queues have the same type
    Exclusive,      // VK: may be used for attachments to preserve DCC on some HW in the cost of making a "queue ownership transfer"

    // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#single-queue-simultaneous-access
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_flags
    Simultaneous    // D3D12: strengthened variant of "CONCURRENT", allowing simultaneous multiple readers and one writer for a texture (requires "Layout::GENERAL")
  };

  enum class MemoryLocation: uint8_t {
    Device,
    DeviceUpload, // soft fallback to "HOST_UPLOAD" if "deviceUploadHeapSize = 0"
    HostUpload,
    HostReadback
  };

  inline DeviceUID constructDeviceUID(uint8_t luid[8], uint8_t uuid[16], bool isLuidValid) {
    DeviceUID out = {};

    if (isLuidValid)
      memcpy(&out.low, luid, sizeof(out.low));
    else {
      memcpy(&out.low, uuid, sizeof(out.low));
      memcpy(&out.high, uuid + 8, sizeof(out.high));
    }

    return out;
  }

  inline bool compareDeviceUID(const DeviceUID& a, const DeviceUID& b) {
    return a.low == b.low && a.high == b.high;
  }

  inline Vendor getVendorFromID(uint32_t vendorID) {
    switch (vendorID) {
      case 0x10DE:
        return Vendor::Nvidia;
      case 0x1002:
        return Vendor::AMD;
      case 0x8086:
        return Vendor::Intel;
      default: break;
    }

    return Vendor::Unknown;
  }

  Result getPhysicalDevices();

  Result createDevice(DeviceCreateInfo createInfo, Device*& device);
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Formats ]
//============================================================================================================================================================================================
// https://docs.vulkan.org/refpages/latest/refpages/source/VkFormat.html
// https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
// left -> right : low -> high bits
// Expected (but not guaranteed) "FormatSupportBits" are provided, but "GetFormatSupport" should be used for querying real HW support
// To demote sRGB use the previous format, i.e. "format - 1"
//                                            STORAGE_WRITE_WITHOUT_FORMAT
//                                           STORAGE_READ_WITHOUT_FORMAT |
//                                                       VERTEX_BUFFER | |
//                                            STORAGE_BUFFER_ATOMICS | | |
//                                                  STORAGE_BUFFER | | | |
//                                                        BUFFER | | | | |
//                                         MULTISAMPLE_RESOLVE | | | | | |
//                                            MULTISAMPLE_8X | | | | | | |
//                                          MULTISAMPLE_4X | | | | | | | |
//                                        MULTISAMPLE_2X | | | | | | | | |
//                                               BLEND | | | | | | | | | |
//                          DEPTH_STENCIL_ATTACHMENT | | | | | | | | | | |
//                                COLOR_ATTACHMENT | | | | | | | | | | | |
//                       STORAGE_TEXTURE_ATOMICS | | | | | | | | | | | | |
//                             STORAGE_TEXTURE | | | | | | | | | | | | | |
//                                   TEXTURE | | | | | | | | | | | | | | |
//                                         | | | | | | | | | | | | | | | |
  enum class Format: uint8_t {
    // |      FormatSupportBits      |
    Unknown,                            // . . . . . . . . . . . . . . . .

    // Plain: 8 bits per channel
    R8_UNORM,                           // + + . + . + + + + + + + . + + +
    R8_SNORM,                           // + + . + . + + + + + + + . + + +
    R8_UINT,                            // + + . + . . + + + . + + . + + +  // SHADING_RATE compatible, see NRI_SHADING_RATE macro
    R8_SINT,                            // + + . + . . + + + . + + . + + +

    RG8_UNORM,                          // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RG8_SNORM,                          // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RG8_UINT,                           // + + . + . . + + + . + + . + + +
    RG8_SINT,                           // + + . + . . + + + . + + . + + +

    BGRA8_UNORM,                        // + + . + . + + + + + + + . + + +
    BGRA8_SRGB,                         // + . . + . + + + + + . . . . . .

    RGBA8_UNORM,                        // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RGBA8_SRGB,                         // + . . + . + + + + + . . . . . .
    RGBA8_SNORM,                        // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RGBA8_UINT,                         // + + . + . . + + + . + + . + + +
    RGBA8_SINT,                         // + + . + . . + + + . + + . + + +

    // Plain: 16 bits per channel
    R16_UNORM,                          // + + . + . + + + + + + + . + + +
    R16_SNORM,                          // + + . + . + + + + + + + . + + +
    R16_UINT,                           // + + . + . . + + + . + + . + + +
    R16_SINT,                           // + + . + . . + + + . + + . + + +
    R16_SFLOAT,                         // + + . + . + + + + + + + . + + +

    RG16_UNORM,                         // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RG16_SNORM,                         // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible
    RG16_UINT,                          // + + . + . . + + + . + + . + + +
    RG16_SINT,                          // + + . + . . + + + . + + . + + +
    RG16_SFLOAT,                        // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible

    RGBA16_UNORM,                       // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    RGBA16_SNORM,                       // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible
    RGBA16_UINT,                        // + + . + . . + + + . + + . + + +
    RGBA16_SINT,                        // + + . + . . + + + . + + . + + +
    RGBA16_SFLOAT,                      // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible

    // Plain: 32 bits per channel
    R32_UINT,                           // + + + + . . + + + . + + + + + +
    R32_SINT,                           // + + + + . . + + + . + + + + + +
    R32_SFLOAT,                         // + + + + . + + + + + + + + + + +

    RG32_UINT,                          // + + . + . . + + + . + + . + + +
    RG32_SINT,                          // + + . + . . + + + . + + . + + +
    RG32_SFLOAT,                        // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible

    RGB32_UINT,                         // + . . . . . . . . . + . . + . .
    RGB32_SINT,                         // + . . . . . . . . . + . . + . .
    RGB32_SFLOAT,                       // + . . . . . . . . + + . . + . .  // "AccelerationStructure" compatible

    RGBA32_UINT,                        // + + . + . . + + + . + + . + + +
    RGBA32_SINT,                        // + + . + . . + + + . + + . + + +
    RGBA32_SFLOAT,                      // + + . + . + + + + + + + . + + +

    // Packed: 16 bits per pixel
    B5_G6_R5_UNORM,                     // + . . + . + + + + + . . . . . .
    B5_G5_R5_A1_UNORM,                  // + . . + . + + + + + . . . . . .
    B4_G4_R4_A4_UNORM,                  // + . . . . . . . . + . . . . . .

    // Packed: 32 bits per pixel
    R10_G10_B10_A2_UNORM,               // + + . + . + + + + + + + . + + +  // "AccelerationStructure" compatible (requires "tiers.rayTracing >= 2")
    R10_G10_B10_A2_UINT,                // + + . + . . + + + . + + . + + +
    R11_G11_B10_UFLOAT,                 // + + . + . + + + + + + + . + + +
    R9_G9_B9_E5_UFLOAT,                 // + . . . . . . . . . . . . . . .

    // Block-compressed (requires "features.textureCompressionBC")
    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/texture-block-compression-in-direct3d-11?source=recommendations
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#S3TC
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#RGTC
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#BPTC
    BC1_RGBA_UNORM,                     // + . . . . . . . . . . . . . . .
    BC1_RGBA_SRGB,                      // + . . . . . . . . . . . . . . .
    BC2_RGBA_UNORM,                     // + . . . . . . . . . . . . . . .
    BC2_RGBA_SRGB,                      // + . . . . . . . . . . . . . . .
    BC3_RGBA_UNORM,                     // + . . . . . . . . . . . . . . .
    BC3_RGBA_SRGB,                      // + . . . . . . . . . . . . . . .
    BC4_R_UNORM,                        // + . . . . . . . . . . . . . . .
    BC4_R_SNORM,                        // + . . . . . . . . . . . . . . .
    BC5_RG_UNORM,                       // + . . . . . . . . . . . . . . .
    BC5_RG_SNORM,                       // + . . . . . . . . . . . . . . .
    BC6H_RGB_UFLOAT,                    // + . . . . . . . . . . . . . . .
    BC6H_RGB_SFLOAT,                    // + . . . . . . . . . . . . . . .
    BC7_RGBA_UNORM,                     // + . . . . . . . . . . . . . . .
    BC7_RGBA_SRGB,                      // + . . . . . . . . . . . . . . .

    // Block-compressed: Ericsson Texture Compression (requires "features.textureCompressionETC2")
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#ETC2
    ETC2_RGB8_UNORM,                    // + . . . . . . . . . . . . . . .
    ETC2_RGB8_SRGB,                     // + . . . . . . . . . . . . . . .
    ETC2_RGB8_A1_UNORM,                 // + . . . . . . . . . . . . . . .
    ETC2_RGB8_A1_SRGB,                  // + . . . . . . . . . . . . . . .
    ETC2_RGB8_A8_UNORM,                 // + . . . . . . . . . . . . . . .
    ETC2_RGB8_A8_SRGB,                  // + . . . . . . . . . . . . . . .
    ETC2_R11_UNORM,                     // + . . . . . . . . . . . . . . .
    ETC2_R11_SNORM,                     // + . . . . . . . . . . . . . . .
    ETC2_R11_G11_UNORM,                 // + . . . . . . . . . . . . . . .
    ETC2_R11_G11_SNORM,                 // + . . . . . . . . . . . . . . .

    // Block-compressed: Adaptive Scalable Texture Compression (requires "features.textureCompressionASTC")
    // https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#ASTC
    ASTC_4X4_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_4X4_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_5X4_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_5X4_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_5X5_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_5X5_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_6X5_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_6X5_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_6X6_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_6X6_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_8X5_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_8X5_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_8X6_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_8X6_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_8X8_UNORM,                     // + . . . . . . . . . . . . . . .
    ASTC_8X8_SRGB,                      // + . . . . . . . . . . . . . . .
    ASTC_10X5_UNORM,                    // + . . . . . . . . . . . . . . .
    ASTC_10X5_SRGB,                     // + . . . . . . . . . . . . . . .
    ASTC_10X6_UNORM,                    // + . . . . . . . . . . . . . . .
    ASTC_10X6_SRGB,                     // + . . . . . . . . . . . . . . .
    ASTC_10X8_UNORM,                    // + . . . . . . . . . . . . . . .
    ASTC_10X8_SRGB,                     // + . . . . . . . . . . . . . . .
    ASTC_10X10_UNORM,                   // + . . . . . . . . . . . . . . .
    ASTC_10X10_SRGB,                    // + . . . . . . . . . . . . . . .
    ASTC_12X10_UNORM,                   // + . . . . . . . . . . . . . . .
    ASTC_12X10_SRGB,                    // + . . . . . . . . . . . . . . .
    ASTC_12X12_UNORM,                   // + . . . . . . . . . . . . . . .
    ASTC_12X12_SRGB,                    // + . . . . . . . . . . . . . . .

    // Depth
    D16_UNORM,                          // + . . . + . + + + . . . . . . .
    D32_SFLOAT,                         // + . . . + . + + + . . . . . . .

    // Depth-stencil
    D24_UNORM_S8_UINT,                  // + . . . + . + + + . . . . . . .
    D32_SFLOAT_S8_UINT,                 // + . . . + . + + + . . . . . . .
    Count                               // Format count
  };

  struct FormatProperties {
    const char* name;       // format name
    Format format;          // self
    uint8_t redBits;        // R (or depth) bits
    uint8_t greenBits;      // G (or stencil) bits (0 if channels < 2)
    uint8_t blueBits;       // B bits (0 if channels < 3)
    uint8_t alphaBits;      // A (or shared exponent) bits (0 if channels < 4)
    uint8_t stride;         // block size in bytes
    uint8_t blockWidth;     // 1 for plain formats, >1 for compressed
    uint8_t blockHeight;    // 1 for plain formats, >1 for compressed
    bool isBgr;             // reversed channels (RGBA => BGRA)
    bool isCompressed;      // block-compressed format
    bool isDepth;           // has depth component
    bool isExpShared;       // shared exponent in alpha channel
    bool isFloat;           // floating point
    bool isPacked;          // 16- or 32- bit packed
    bool isInteger;         // integer
    bool isNorm;            // [0; 1] normalized
    bool isSigned;          // signed
    bool isSrgb;            // sRGB
    bool isStencil;         // has stencil component
  };

  // A bit represents a feature, supported by a format
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_feature_data_format_support
  // https://docs.vulkan.org/refpages/latest/refpages/source/VkFormatFeatureFlagBits2.html
  // WGPU: typed buffer views are unsupported; storage textures cannot be multisampled
  ENGINE_ENUM_BITS(FormatSupportBits, uint16_t,
    Unsupported                     = 0,            // format is unsupported

    // Texture
    Texture                         = ENGINE_BIT(0),    // sampled texture view
    StorageTexture                  = ENGINE_BIT(1),    // storage texture view
    StorageTextureAtomics           = ENGINE_BIT(2),    // storage texture atomics other than Load / Store
    ColorAttachment                 = ENGINE_BIT(3),    // color attachment view
    DepthStencilAttachment          = ENGINE_BIT(4),    // depth-stencil attachment view
    Blend                           = ENGINE_BIT(5),    // color attachment blending
    Multisample2X                   = ENGINE_BIT(6),    // 2x multisampled texture
    MultiSample4X                   = ENGINE_BIT(7),    // 4x multisampled texture
    MultiSample8X                   = ENGINE_BIT(8),    // 8x multisampled texture
    MultiSampleResolve              = ENGINE_BIT(9),    // resolve source/destination

    // Buffer
    Buffer                          = ENGINE_BIT(10),   // typed buffer view
    StorageBuffer                   = ENGINE_BIT(11),   // typed storage buffer view
    StorageBufferAtomics            = ENGINE_BIT(12),   // typed storage buffer atomics other than Load / Store
    VertexBuffer                    = ENGINE_BIT(13),   // vertex buffer attribute

    // Texture / buffer
    StorageReadWithoutFormat        = ENGINE_BIT(14),   // storage read with unknown format
    StorageWriteWithoutFormat       = ENGINE_BIT(15)    // storage write with unknown format
  );

#define _ 0
#define X 1
  constexpr std::array<FormatProperties, static_cast<size_t>(Format::Count)> formatProperties = {{
    //                                                                                                               isStencil
    //                                                                                                               isSrgb  |
    //                                                                                                          isSigned  |  |
    //                                                                                                         isNorm  |  |  |
    //                                                                                                   isInteger  |  |  |  |
    //                                                                                                 isPacked  |  |  |  |  |
    //                                                                                               isFloat  |  |  |  |  |  |
    //                                                                                        isExpShared  |  |  |  |  |  |  |
    //                                                                                         isDepth  |  |  |  |  |  |  |  |
    //                                                                                 isCompressed  |  |  |  |  |  |  |  |  |
    //                                                                                     isBgr  |  |  |  |  |  |  |  |  |  |
    //                                                                           blockHeight   |  |  |  |  |  |  |  |  |  |  |
    //                                                                        blockWidth   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                                        stride   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                                    A bits   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                                B bits   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                            G bits   |   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                                                        R bits   |   |   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    //                          self                               |   |   |   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    // format name              |                                  |   |   |   |   |   |   |   |  |  |  |  |  |  |  |  |  |  |
    {"Unknown",                 Format::Unknown,                   0,  0,  0,  0,  1,  0,  0,  _, _, _, _, _, _, _, _, _, _, _}, // UNKNOWN
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"R8_UNORM",                Format::R8_UNORM,                  8,  0,  0,  0,  1,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // R8_UNORM
    {"R8_SNORM",                Format::R8_SNORM,                  8,  0,  0,  0,  1,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // R8_SNORM
    {"R8_UINT",                 Format::R8_UINT,                   8,  0,  0,  0,  1,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // R8_UINT
    {"R8_SINT",                 Format::R8_SINT,                   8,  0,  0,  0,  1,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // R8_SINT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RG8_UNORM",               Format::RG8_UNORM,                 8,  8,  0,  0,  2,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // RG8_UNORM
    {"RG8_SNORM",               Format::RG8_SNORM,                 8,  8,  0,  0,  2,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // RG8_SNORM
    {"RG8_UINT",                Format::RG8_UINT,                  8,  8,  0,  0,  2,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RG8_UINT
    {"RG8_SINT",                Format::RG8_SINT,                  8,  8,  0,  0,  2,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RG8_SINT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"BGRA8_UNORM",             Format::BGRA8_UNORM,               8,  8,  8,  8,  4,  1,  1,  X, _, _, _, _, _, _, X, _, _, _}, // BGRA8_UNORM
    {"BGRA8_SRGB",              Format::BGRA8_SRGB,                8,  8,  8,  8,  4,  1,  1,  X, _, _, _, _, _, _, _, _, X, _}, // BGRA8_SRGB
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RGBA8_UNORM",             Format::RGBA8_UNORM,               8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // RGBA8_UNORM
    {"RGBA8_SRGB",              Format::RGBA8_SRGB,                8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, _, _, _, X, _}, // RGBA8_SRGB
    {"RGBA8_SNORM",             Format::RGBA8_SNORM,               8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // RGBA8_SNORM
    {"RGBA8_UINT",              Format::RGBA8_UINT,                8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RGBA8_UINT
    {"RGBA8_SINT",              Format::RGBA8_SINT,                8,  8,  8,  8,  4,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RGBA8_SINT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"R16_UNORM",               Format::R16_UNORM,                 16, 0,  0,  0,  2,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // R16_UNORM
    {"R16_SNORM",               Format::R16_SNORM,                 16, 0,  0,  0,  2,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // R16_SNORM
    {"R16_UINT",                Format::R16_UINT,                  16, 0,  0,  0,  2,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // R16_UINT
    {"R16_SINT",                Format::R16_SINT,                  16, 0,  0,  0,  2,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // R16_SINT
    {"R16_SFLOAT",              Format::R16_SFLOAT,                16, 0,  0,  0,  2,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // R16_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RG16_UNORM",              Format::RG16_UNORM,                16, 16, 0,  0,  4,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // RG16_UNORM
    {"RG16_SNORM",              Format::RG16_SNORM,                16, 16, 0,  0,  4,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // RG16_SNORM
    {"RG16_UINT",               Format::RG16_UINT,                 16, 16, 0,  0,  4,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RG16_UINT
    {"RG16_SINT",               Format::RG16_SINT,                 16, 16, 0,  0,  4,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RG16_SINT
    {"RG16_SFLOAT",             Format::RG16_SFLOAT,               16, 16, 0,  0,  4,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RG16_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RGBA16_UNORM",            Format::RGBA16_UNORM,              16, 16, 16, 16, 8,  1,  1,  _, _, _, _, _, _, _, X, _, _, _}, // RGBA16_UNORM
    {"RGBA16_SNORM",            Format::RGBA16_SNORM,              16, 16, 16, 16, 8,  1,  1,  _, _, _, _, _, _, _, X, X, _, _}, // RGBA16_SNORM
    {"RGBA16_UINT",             Format::RGBA16_UINT,               16, 16, 16, 16, 8,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RGBA16_UINT
    {"RGBA16_SINT",             Format::RGBA16_SINT,               16, 16, 16, 16, 8,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RGBA16_SINT
    {"RGBA16_SFLOAT",           Format::RGBA16_SFLOAT,             16, 16, 16, 16, 8,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RGBA16_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"R32_UINT",                Format::R32_UINT,                  32, 0,  0,  0,  4,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // R32_UINT
    {"R32_SINT",                Format::R32_SINT,                  32, 0,  0,  0,  4,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // R32_SINT
    {"R32_SFLOAT",              Format::R32_SFLOAT,                32, 0,  0,  0,  4,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // R32_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RG32_UINT",               Format::RG32_UINT,                 32, 32, 0,  0,  8,  1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RG32_UINT
    {"RG32_SINT",               Format::RG32_SINT,                 32, 32, 0,  0,  8,  1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RG32_SINT
    {"RG32_SFLOAT",             Format::RG32_SFLOAT,               32, 32, 0,  0,  8,  1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RG32_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RGB32_UINT",              Format::RGB32_UINT,                32, 32, 32, 0,  12, 1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RGB32_UINT
    {"RGB32_SINT",              Format::RGB32_SINT,                32, 32, 32, 0,  12, 1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RGB32_SINT
    {"RGB32_SFLOAT",            Format::RGB32_SFLOAT,              32, 32, 32, 0,  12, 1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RGB32_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"RGBA32_UINT",             Format::RGBA32_UINT,               32, 32, 32, 32, 16, 1,  1,  _, _, _, _, _, _, X, _, _, _, _}, // RGBA32_UINT
    {"RGBA32_SINT",             Format::RGBA32_SINT,               32, 32, 32, 32, 16, 1,  1,  _, _, _, _, _, _, X, _, X, _, _}, // RGBA32_SINT
    {"RGBA32_SFLOAT",           Format::RGBA32_SFLOAT,             32, 32, 32, 32, 16, 1,  1,  _, _, _, _, X, _, _, _, X, _, _}, // RGBA32_SFLOAT
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"B5_G6_R5_UNORM",          Format::B5_G6_R5_UNORM,            5,  6,  5,  0,  2,  1,  1,  X, _, _, _, _, X, _, X, _, _, _}, // B5_G6_R5_UNORM
    {"B5_G5_R5_A1_UNORM",       Format::B5_G5_R5_A1_UNORM,         5,  5,  5,  1,  2,  1,  1,  X, _, _, _, _, X, _, X, _, _, _}, // B5_G5_R5_A1_UNORM
    {"B4_G4_R4_A4_UNORM",       Format::B4_G4_R4_A4_UNORM,         4,  4,  4,  4,  2,  1,  1,  X, _, _, _, _, X, _, X, _, _, _}, // B4_G4_R4_A4_UNORM
    {"R10_G10_B10_A2_UNORM",    Format::R10_G10_B10_A2_UNORM,      10, 10, 10, 2,  4,  1,  1,  _, _, _, _, _, X, _, X, _, _, _}, // R10_G10_B10_A2_UNORM
    {"R10_G10_B10_A2_UINT",     Format::R10_G10_B10_A2_UINT,       10, 10, 10, 2,  4,  1,  1,  _, _, _, _, _, X, X, _, _, _, _}, // R10_G10_B10_A2_UINT
    {"R11_G11_B10_UFLOAT",      Format::R11_G11_B10_UFLOAT,        11, 11, 10, 0,  4,  1,  1,  _, _, _, _, X, X, _, _, _, _, _}, // R11_G11_B10_UFLOAT
    {"R9_G9_B9_E5_UFLOAT",      Format::R9_G9_B9_E5_UFLOAT,        9,  9,  9,  5,  4,  1,  1,  _, _, _, X, X, X, _, _, _, _, _}, // R9_G9_B9_E5_UFLOAT
    //                                                             r   g   b   a   s   w   h   b   c  d  e  f  p  i  n  s  s  s
    {"BC1_RGBA_UNORM",          Format::BC1_RGBA_UNORM,            5,  6,  5,  1,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC1_RGBA_UNORM
    {"BC1_RGBA_SRGB",           Format::BC1_RGBA_SRGB,             5,  6,  5,  1,  8,  4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // BC1_RGBA_SRGB
    {"BC2_RGBA_UNORM",          Format::BC2_RGBA_UNORM,            5,  6,  5,  4,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC2_RGBA_UNORM
    {"BC2_RGBA_SRGB",           Format::BC2_RGBA_SRGB,             5,  6,  5,  4,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // BC2_RGBA_SRGB
    {"BC3_RGBA_UNORM",          Format::BC3_RGBA_UNORM,            5,  6,  5,  8,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC3_RGBA_UNORM
    {"BC3_RGBA_SRGB",           Format::BC3_RGBA_SRGB,             5,  6,  5,  8,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // BC3_RGBA_SRGB
    {"BC4_R_UNORM",             Format::BC4_R_UNORM,               8,  0,  0,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC4_R_UNORM
    {"BC4_R_SNORM",             Format::BC4_R_SNORM,               8,  0,  0,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, X, _, _}, // BC4_R_SNORM
    {"BC5_RG_UNORM",            Format::BC5_RG_UNORM,              8,  8,  0,  0,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC5_RG_UNORM
    {"BC5_RG_SNORM",            Format::BC5_RG_SNORM,              8,  8,  0,  0,  16, 4,  4,  _, X, _, _, _, _, _, X, X, _, _}, // BC5_RG_SNORM
    {"BC6H_RGB_UFLOAT",         Format::BC6H_RGB_UFLOAT,           16, 16, 16, 0,  16, 4,  4,  _, X, _, _, X, _, _, _, _, _, _}, // BC6H_RGB_UFLOAT
    {"BC6H_RGB_SFLOAT",         Format::BC6H_RGB_SFLOAT,           16, 16, 16, 0,  16, 4,  4,  _, X, _, _, X, _, _, _, X, _, _}, // BC6H_RGB_SFLOAT
    {"BC7_RGBA_UNORM",          Format::BC7_RGBA_UNORM,            8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // BC7_RGBA_UNORM
    {"BC7_RGBA_SRGB",           Format::BC7_RGBA_SRGB,             8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // BC7_RGBA_SRGB
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"ETC2_RGB8_UNORM",         Format::ETC2_RGB8_UNORM,           8,  8,  8,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_RGB8_UNORM
    {"ETC2_RGB8_SRGB",          Format::ETC2_RGB8_SRGB,            8,  8,  8,  0,  8,  4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ETC2_RGB8_SRGB
    {"ETC2_RGB8_A1_UNORM",      Format::ETC2_RGB8_A1_UNORM,        8,  8,  8,  1,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_RGB8_A1_UNORM
    {"ETC2_RGB8_A1_SRGB",       Format::ETC2_RGB8_A1_SRGB,         8,  8,  8,  1,  8,  4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ETC2_RGB8_A1_SRGB
    {"ETC2_RGB8_A8_UNORM",      Format::ETC2_RGB8_A8_UNORM,        8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_RGB8_A8_UNORM
    {"ETC2_RGB8_A8_SRGB",       Format::ETC2_RGB8_A8_SRGB,         8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ETC2_RGB8_A8_SRGB
    {"ETC2_R11_UNORM",          Format::ETC2_R11_UNORM,            11, 0,  0,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_R11_UNORM
    {"ETC2_R11_SNORM",          Format::ETC2_R11_SNORM,            11, 0,  0,  0,  8,  4,  4,  _, X, _, _, _, _, _, X, X, _, _}, // ETC2_R11_SNORM
    {"ETC2_R11_G11_UNORM",      Format::ETC2_R11_G11_UNORM,        11, 11, 0,  0,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ETC2_R11_G11_UNORM
    {"ETC2_R11_G11_SNORM",      Format::ETC2_R11_G11_SNORM,        11, 11, 0,  0,  16, 4,  4,  _, X, _, _, _, _, _, X, X, _, _}, // ETC2_R11_G11_SNORM
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"ASTC_4X4_UNORM",          Format::ASTC_4X4_UNORM,            8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_4X4_UNORM
    {"ASTC_4X4_SRGB",           Format::ASTC_4X4_SRGB,             8,  8,  8,  8,  16, 4,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_4X4_SRGB
    {"ASTC_5X4_UNORM",          Format::ASTC_5X4_UNORM,            8,  8,  8,  8,  16, 5,  4,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_5X4_UNORM
    {"ASTC_5X4_SRGB",           Format::ASTC_5X4_SRGB,             8,  8,  8,  8,  16, 5,  4,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_5X4_SRGB
    {"ASTC_5X5_UNORM",          Format::ASTC_5X5_UNORM,            8,  8,  8,  8,  16, 5,  5,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_5X5_UNORM
    {"ASTC_5X5_SRGB",           Format::ASTC_5X5_SRGB,             8,  8,  8,  8,  16, 5,  5,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_5X5_SRGB
    {"ASTC_6X5_UNORM",          Format::ASTC_6X5_UNORM,            8,  8,  8,  8,  16, 6,  5,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_6X5_UNORM
    {"ASTC_6X5_SRGB",           Format::ASTC_6X5_SRGB,             8,  8,  8,  8,  16, 6,  5,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_6X5_SRGB
    {"ASTC_6X6_UNORM",          Format::ASTC_6X6_UNORM,            8,  8,  8,  8,  16, 6,  6,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_6X6_UNORM
    {"ASTC_6X6_SRGB",           Format::ASTC_6X6_SRGB,             8,  8,  8,  8,  16, 6,  6,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_6X6_SRGB
    {"ASTC_8X5_UNORM",          Format::ASTC_8X5_UNORM,            8,  8,  8,  8,  16, 8,  5,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_8X5_UNORM
    {"ASTC_8X5_SRGB",           Format::ASTC_8X5_SRGB,             8,  8,  8,  8,  16, 8,  5,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_8X5_SRGB
    {"ASTC_8X6_UNORM",          Format::ASTC_8X6_UNORM,            8,  8,  8,  8,  16, 8,  6,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_8X6_UNORM
    {"ASTC_8X6_SRGB",           Format::ASTC_8X6_SRGB,             8,  8,  8,  8,  16, 8,  6,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_8X6_SRGB
    {"ASTC_8X8_UNORM",          Format::ASTC_8X8_UNORM,            8,  8,  8,  8,  16, 8,  8,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_8X8_UNORM
    {"ASTC_8X8_SRGB",           Format::ASTC_8X8_SRGB,             8,  8,  8,  8,  16, 8,  8,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_8X8_SRGB
    {"ASTC_10X5_UNORM",         Format::ASTC_10X5_UNORM,           8,  8,  8,  8,  16, 10, 5,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_10X5_UNORM
    {"ASTC_10X5_SRGB",          Format::ASTC_10X5_SRGB,            8,  8,  8,  8,  16, 10, 5,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_10X5_SRGB
    {"ASTC_10X6_UNORM",         Format::ASTC_10X6_UNORM,           8,  8,  8,  8,  16, 10, 6,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_10X6_UNORM
    {"ASTC_10X6_SRGB",          Format::ASTC_10X6_SRGB,            8,  8,  8,  8,  16, 10, 6,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_10X6_SRGB
    {"ASTC_10X8_UNORM",         Format::ASTC_10X8_UNORM,           8,  8,  8,  8,  16, 10, 8,  _, X, _, _, _, _, _, X, _, _, _}, // ASTC_10X8_UNORM
    {"ASTC_10X8_SRGB",          Format::ASTC_10X8_SRGB,            8,  8,  8,  8,  16, 10, 8,  _, X, _, _, _, _, _, _, _, X, _}, // ASTC_10X8_SRGB
    {"ASTC_10X10_UNORM",        Format::ASTC_10X10_UNORM,          8,  8,  8,  8,  16, 10, 10, _, X, _, _, _, _, _, X, _, _, _}, // ASTC_10X10_UNORM
    {"ASTC_10X10_SRGB",         Format::ASTC_10X10_SRGB,           8,  8,  8,  8,  16, 10, 10, _, X, _, _, _, _, _, _, _, X, _}, // ASTC_10X10_SRGB
    {"ASTC_12X10_UNORM",        Format::ASTC_12X10_UNORM,          8,  8,  8,  8,  16, 12, 10, _, X, _, _, _, _, _, X, _, _, _}, // ASTC_12X10_UNORM
    {"ASTC_12X10_SRGB",         Format::ASTC_12X10_SRGB,           8,  8,  8,  8,  16, 12, 10, _, X, _, _, _, _, _, _, _, X, _}, // ASTC_12X10_SRGB
    {"ASTC_12X12_UNORM",        Format::ASTC_12X12_UNORM,          8,  8,  8,  8,  16, 12, 12, _, X, _, _, _, _, _, X, _, _, _}, // ASTC_12X12_UNORM
    {"ASTC_12X12_SRGB",         Format::ASTC_12X12_SRGB,           8,  8,  8,  8,  16, 12, 12, _, X, _, _, _, _, _, _, _, X, _}, // ASTC_12X12_SRGB
    //                                                             r   g   b   a   s   w   h   b  c  d  e  f  p  i  n  s  s  s
    {"D16_UNORM",               Format::D16_UNORM,                 16, 0,  0,  0,  2,  1,  1,  _, _, X, _, _, _, _, X, _, _, _}, // D16_UNORM
    {"D32_SFLOAT",              Format::D32_SFLOAT,                32, 0,  0,  0,  4,  1,  1,  _, _, X, _, X, _, _, _, X, _, _}, // D32_SFLOAT
    {"D24_UNORM_S8_UINT",       Format::D24_UNORM_S8_UINT,         24, 8,  0,  0,  4,  1,  1,  _, _, X, _, _, _, X, X, _, _, X}, // D24_UNORM_S8_UINT
    {"D32_SFLOAT_S8_UINT",      Format::D32_SFLOAT_S8_UINT,        32, 8,  0,  0,  8,  1,  1,  _, _, X, _, X, _, X, _, X, _, X}, // D32_SFLOAT_S8_UINT
  }};
#undef _
#undef X

  inline const FormatProperties &getFormatProperties(Format format) {
    return formatProperties[static_cast<size_t>(format)];
  }
#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Resources: creation ]
//============================================================================================================================================================================================
  class Texture: public Resource {
    public:
      Texture() = default;

      ~Texture() override = default;
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageType.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_dimension
  enum class TextureDimension: uint8_t {
    Dimension1D,
    Dimension2D,
    Dimension3D
  };

  // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageUsageFlagBits.html
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_flags
  ENGINE_ENUM_BITS(TextureUsageBits, uint8_t,             // Min compatible access:                   Usage:
    None                                = 0,
    ShaderResource                      = ENGINE_BIT(0),    // ShaderResource                           Read-only shader resource view (SRV)
    ShaderResourceStorage               = ENGINE_BIT(1),    // ShaderResourceStorage                    Read/write shader resource view (UAV)
    ColorAttachment                     = ENGINE_BIT(2),    // ColorAttachment                          Color attachment (render target)
    DepthStencilAttachment              = ENGINE_BIT(3),    // DepthStencilAttachmentRead/Write         Depth-stencil attachment (depth-stencil target)
    ShadingRateAttachment               = ENGINE_BIT(4),    // ShadingRateAttachment                    Shading rate attachment (source)
    InputAttachment                     = ENGINE_BIT(5)     // InputAttachment                          Subpass input (read on-chip tile cache)
  );

  struct TextureInfo {
    TextureDimension dimension;
    TextureUsageBits usage;
    Format format;
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t mipNum;
    uint16_t layerNum;
    uint8_t sampleNum;
    SharingMode sharingMode;
    ClearValue optimizedClearValue;    // D3D12: not needed on desktop, since any HW can track many clear values
  };
#pragma endregion
  /* clang-format on */
}
