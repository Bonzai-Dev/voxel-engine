// © 2021 NVIDIA Corporation

// Goal: utilities

#pragma once

#define NRI_HELPER_H 1

namespace Core::RHI {
  struct VideoMemoryInfo {
    uint64_t budgetSize;
    // the OS-provided video memory budget. If "usageSize" > "budgetSize", the application may incur stuttering or performance penalties
    uint64_t usageSize; // specifies the application’s current video memory usage
  };

  struct TextureSubresourceUploadDesc {
    const void *slices;
    uint32_t sliceNum;
    uint32_t rowPitch;
    uint32_t slicePitch;
  };

  struct TextureUploadDesc {
    RHI_OPTIONAL const TextureSubresourceUploadDesc *subresources;
    // if provided, must include ALL subresources = layerNum * mipNum
    Texture *texture;
    AccessLayoutStage after;
    PlaneBits planes;
  };

  struct BufferUploadDesc {
    RHI_OPTIONAL const void *data; // if provided, must be data for the whole buffer
    Buffer *buffer;
    AccessStage after;
  };

  struct ResourceGroupDesc {
    MemoryLocation memoryLocation;
    Texture *const*textures;
    uint32_t textureNum;
    Buffer *const*buffers;
    uint32_t bufferNum;
    RHI_OPTIONAL uint64_t preferredMemorySize;
    // desired chunk size (but can be greater if a resource doesn't fit), 256 Mb if 0
    RHI_OPTIONAL float residencyPriority; // [-1; 1]: low < 0, normal = 0, high > 0
  };

  struct FormatProps {
    const char *name; // format name
    Format format; // self
    uint8_t redBits; // R (or depth) bits
    uint8_t greenBits; // G (or stencil) bits (0 if channels < 2)
    uint8_t blueBits; // B bits (0 if channels < 3)
    uint8_t alphaBits; // A (or shared exponent) bits (0 if channels < 4)
    uint8_t stride; // block size in bytes
    uint8_t blockWidth; // 1 for plain formats, >1 for compressed
    uint8_t blockHeight; // 1 for plain formats, >1 for compressed
    bool isBgr; // reversed channels (RGBA => BGRA)
    bool isCompressed; // block-compressed format
    bool isDepth; // has depth component
    bool isExpShared; // shared exponent in alpha channel
    bool isFloat; // floating point
    bool isPacked; // 16- or 32- bit packed
    bool isInteger; // integer
    bool isNorm; // [0; 1] normalized
    bool isSigned; // signed
    bool isSrgb; // sRGB
    bool isStencil; // has stencil component
  };

  /*clang-format off*/
  // Threadsafe: yes
  struct HelperInterface {
    // Optimized memory allocation for a group of resources
    uint32_t    (*CalculateAllocationNumber)   (const Device &Device, const ResourceGroupDesc &ResourceGroupDesc);
    Result (*AllocateAndBindMemory)       (Device &Device, const ResourceGroupDesc &ResourceGroupDesc, RHI_OUT Memory** allocations); // "allocations" must have entries >= returned by "CalculateAllocationNumber"

    // Populate resources with data (not for streaming!)
    Result (*UploadData)                  (Queue &Queue, const TextureUploadDesc *TextureUploadDescs, uint32_t textureUploadDescNum, const BufferUploadDesc *BufferUploadDescs, uint32_t bufferUploadDescNum);

    // Information about video memory
    Result (*QueryVideoMemoryInfo)        (const Device &Device, MemoryLocation memoryLocation, RHI_OUT VideoMemoryInfo &VideoMemoryInfo);
  };
  /*clang-format on*/

  // Format utilities
  Format nriConvertDXGIFormatToNRI(uint32_t dxgiFormat); // returns best-matched typed format for "TYPELESS"
  Format nriConvertVKFormatToNRI(uint32_t vkFormat);
  uint32_t nriConvertNRIFormatToDXGI(Format format);
  uint32_t nriConvertNRIFormatToVK(Format format);
  const FormatProps *nriGetFormatProps(Format format);

  // Strings
  const char *nriGetGraphicsAPIString(GraphicsBackend graphicsAPI);

  // A friendly way to get a supported depth format
  static inline Format GetSupportedDepthFormat(
    const CoreInterface &coreInterface,
    const Device &device,
    uint32_t minBits,
    bool stencil
  ) {
    if (minBits <= 16 && !stencil) {
      if (coreInterface.getFormatSupport(device, Format::D16_UNORM) & FormatSupportBits::DepthStencilAttachment)
        return Format::D16_UNORM;
    }

    if (minBits <= 24) {
      if (coreInterface.getFormatSupport(device, Format::D24_UNORM_S8_UINT) &
        FormatSupportBits::DepthStencilAttachment)
        return Format::D24_UNORM_S8_UINT;
    }

    if (minBits <= 32 && !stencil) {
      if (coreInterface.getFormatSupport(device, Format::D32_SFLOAT) & FormatSupportBits::DepthStencilAttachment)
        return Format::D32_SFLOAT;
    }

    if (coreInterface.getFormatSupport(device, Format::D32_SFLOAT_S8_UINT) &
      FormatSupportBits::DepthStencilAttachment)
      return Format::D32_SFLOAT_S8_UINT;

    // Should be unreachable
    return Format::Unknown;
  }

  // A convinient way to fit pipeline layout settings into the device limits, respecting various restrictions
  struct PipelineLayoutSettingsDesc {
    uint32_t descriptorSetNum;
    uint32_t descriptorRangeNum;
    uint32_t rootConstantSize;
    uint32_t rootDescriptorNum;
    bool preferRootDescriptorsOverConstants;

    // D3D12 only (see "NRI.hlsl" for more details)
    bool enableD3D12DrawParametersEmulation;
    bool enableD3D12DrawIndexEmulation;
  };

  static inline PipelineLayoutSettingsDesc FitPipelineLayoutSettingsIntoDeviceLimits(
    const DeviceInfo &deviceDesc,
    const PipelineLayoutSettingsDesc &pipelineLayoutSettingsDesc
  ) {
    uint32_t descriptorSetNum = pipelineLayoutSettingsDesc.descriptorSetNum;
    uint32_t descriptorRangeNum = pipelineLayoutSettingsDesc.descriptorRangeNum;
    uint32_t rootConstantSize = pipelineLayoutSettingsDesc.rootConstantSize;
    uint32_t rootDescriptorNum = pipelineLayoutSettingsDesc.rootDescriptorNum;

    // Apply global limits
    if (rootConstantSize > deviceDesc.pipelineLayout.rootConstantMaxSize)
      rootConstantSize = deviceDesc.pipelineLayout.rootConstantMaxSize;

    if (rootDescriptorNum > deviceDesc.pipelineLayout.rootDescriptorMaxNum)
      rootDescriptorNum = deviceDesc.pipelineLayout.rootDescriptorMaxNum;

    uint32_t pipelineLayoutDescriptorSetMaxNum = deviceDesc.pipelineLayout.descriptorSetMaxNum;

    // D3D12 has limited-size root signature
    if (deviceDesc.graphicsBackend == GraphicsBackend::D3D12) {
      const uint32_t descriptorTableCost = 4;
      const uint32_t rootDescriptorCost = 8;

      uint32_t freeBytesInRootSignature = 256;

      // Reserved 1 root descriptor for "draw parameters" emulation
      if (pipelineLayoutSettingsDesc.enableD3D12DrawParametersEmulation)
        freeBytesInRootSignature -= 8;

      // Reserved 1 root constant for "draw index" emulation
      if (pipelineLayoutSettingsDesc.enableD3D12DrawIndexEmulation)
        freeBytesInRootSignature -= 4;

      // Must fit
      uint32_t availableDescriptorRangeNum = freeBytesInRootSignature / descriptorTableCost;
      if (descriptorRangeNum > availableDescriptorRangeNum)
        descriptorRangeNum = availableDescriptorRangeNum;

      freeBytesInRootSignature -= descriptorRangeNum * descriptorTableCost;

      // Desired fit
      if (pipelineLayoutSettingsDesc.preferRootDescriptorsOverConstants) {
        uint32_t availableRootDescriptorNum = freeBytesInRootSignature / rootDescriptorCost;
        if (rootDescriptorNum > availableRootDescriptorNum)
          rootDescriptorNum = availableRootDescriptorNum;

        freeBytesInRootSignature -= rootDescriptorNum * rootDescriptorCost;

        if (rootConstantSize > freeBytesInRootSignature)
          rootConstantSize = freeBytesInRootSignature;
      }
      else {
        if (rootConstantSize > freeBytesInRootSignature)
          rootConstantSize = freeBytesInRootSignature;

        freeBytesInRootSignature -= rootConstantSize;

        uint32_t availableRootDescriptorNum = freeBytesInRootSignature / rootDescriptorCost;
        if (rootDescriptorNum > availableRootDescriptorNum)
          rootDescriptorNum = availableRootDescriptorNum;
      }
    }
    else if (rootDescriptorNum)
      pipelineLayoutDescriptorSetMaxNum--;

    if (descriptorSetNum > pipelineLayoutDescriptorSetMaxNum)
      descriptorSetNum = pipelineLayoutDescriptorSetMaxNum;

    PipelineLayoutSettingsDesc modifiedPipelineLayoutLimitsDesc = *&pipelineLayoutSettingsDesc;
    modifiedPipelineLayoutLimitsDesc.descriptorSetNum = descriptorSetNum;
    modifiedPipelineLayoutLimitsDesc.descriptorRangeNum = descriptorRangeNum;
    modifiedPipelineLayoutLimitsDesc.rootConstantSize = rootConstantSize;
    modifiedPipelineLayoutLimitsDesc.rootDescriptorNum = rootDescriptorNum;

    return modifiedPipelineLayoutLimitsDesc;
  }
}
