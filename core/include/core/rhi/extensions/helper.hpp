// Goal: utilities

#pragma once
#include <cstddef>
#include <cstdint>
#include <core/rhi/rhi.hpp>

#define NRI_HELPER_H 1

namespace Core::RHI {
    struct VideoMemoryInfo {
        uint64_t budgetSize;    // the OS-provided video memory budget. If "usageSize" > "budgetSize", the application may incur stuttering or performance penalties
        uint64_t usageSize;     // specifies the application’s current video memory usage
    };

    struct TextureSubresourceUploadDesc {
        const void* slices;
        uint32_t sliceNum;
        uint32_t rowPitch;
        uint32_t slicePitch;
    };

    struct TextureUploadDesc {
        const TextureSubresourceUploadDesc *subresources; // if provided, must include ALL subresources = layerNum * mipNum
        Texture *texture;
        AccessLayoutStage after;
        PlaneBits planes;
    };

    struct BufferUploadDesc {
        const void* data; // if provided, must be data for the whole buffer
        Buffer *buffer;
        AccessStage after;
    };

    struct ResourceGroupDesc {
        MemoryLocation memoryLocation;
        Texture *const* textures;
        uint32_t textureNum;
        Buffer *const* buffers;
        uint32_t bufferNum;
        uint64_t preferredMemorySize; // desired chunk size (but can be greater if a resource doesn't fit), 256 Mb if 0
        float residencyPriority; // [-1; 1]: low < 0, normal = 0, high > 0
    };

    struct FormatProps {
        const char* name;       // format name
        DataFormat format;     // self
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

    // Threadsafe: yes
    struct HelperInterface {
        // Optimized memory allocation for a group of resources
        uint32_t    (*CalculateAllocationNumber)   (const Device &device, const ResourceGroupDesc &resourceGroupDesc);
        Result (*AllocateAndBindMemory)       (Device &device, const ResourceGroupDesc &resourceGroupDesc, Memory** allocations); // "allocations" must have entries >= returned by "CalculateAllocationNumber"

        // Populate resources with data (not for streaming!)
        Result (*UploadData)                  (Queue &queue, const TextureUploadDesc *textureUploadDescs, uint32_t textureUploadDescNum, const BufferUploadDesc *bufferUploadDescs, uint32_t bufferUploadDescNum);

        // Information about video memory
        Result (*QueryVideoMemoryInfo)        (const Device &device, MemoryLocation memoryLocation, VideoMemoryInfo &videoMemoryInfo);
    };

    // Format utilities
    DataFormat nriConvertDXGIFormatToNRI(uint32_t dxgiFormat); // returns best-matched typed format for "TYPELESS"
    DataFormat nriConvertVKFormatToNRI(uint32_t vkFormat);
    uint32_t nriConvertNRIFormatToDXGI(DataFormat format);
    uint32_t nriConvertNRIFormatToVK(DataFormat format);
    const FormatProps *nriGetFormatProps(DataFormat format);

    // Strings
    const char* nriGetGraphicsAPIString(GraphicsBackend graphicsAPI);

    // A friendly way to get a supported depth format
    static inline DataFormat GetSupportedDepthFormat(const CoreInterface &coreInterface, const Device &device, uint32_t minBits, bool stencil) {
        if (minBits <= 16 && !stencil) {
            if (NriDeref(coreInterface)->GetFormatSupport(device, NriScopedMember(Format, D16_UNORM)) & NriScopedMember(FormatSupportBits, DEPTH_STENCIL_ATTACHMENT))
                return NriScopedMember(Format, D16_UNORM);
        }

        if (minBits <= 24) {
            if (NriDeref(coreInterface)->GetFormatSupport(device, NriScopedMember(Format, D24_UNORM_S8_UINT)) & NriScopedMember(FormatSupportBits, DEPTH_STENCIL_ATTACHMENT))
                return NriScopedMember(Format, D24_UNORM_S8_UINT);
        }

        if (minBits <= 32 && !stencil) {
            if (NriDeref(coreInterface)->GetFormatSupport(device, NriScopedMember(Format, D32_SFLOAT)) & NriScopedMember(FormatSupportBits, DEPTH_STENCIL_ATTACHMENT))
                return NriScopedMember(Format, D32_SFLOAT);
        }

        if (NriDeref(coreInterface)->GetFormatSupport(device, NriScopedMember(Format, D32_SFLOAT_S8_UINT)) & NriScopedMember(FormatSupportBits, DEPTH_STENCIL_ATTACHMENT))
            return NriScopedMember(Format, D32_SFLOAT_S8_UINT);

        // Should be unreachable
        return NriScopedMember(Format, UNKNOWN);
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

    static inline Nri(PipelineLayoutSettingsDesc) NriFunc(FitPipelineLayoutSettingsIntoDeviceLimits)(const NriRef(DeviceInfo) DeviceInfo, const NriRef(PipelineLayoutSettingsDesc) pipelineLayoutSettingsDesc) {
        uint32_t descriptorSetNum = NriDeref(pipelineLayoutSettingsDesc)->descriptorSetNum;
        uint32_t descriptorRangeNum = NriDeref(pipelineLayoutSettingsDesc)->descriptorRangeNum;
        uint32_t rootConstantSize = NriDeref(pipelineLayoutSettingsDesc)->rootConstantSize;
        uint32_t rootDescriptorNum = NriDeref(pipelineLayoutSettingsDesc)->rootDescriptorNum;

        // Apply global limits
        if (rootConstantSize > NriDeref(DeviceInfo)->pipelineLayout.rootConstantMaxSize)
            rootConstantSize = NriDeref(DeviceInfo)->pipelineLayout.rootConstantMaxSize;

        if (rootDescriptorNum > NriDeref(DeviceInfo)->pipelineLayout.rootDescriptorMaxNum)
            rootDescriptorNum = NriDeref(DeviceInfo)->pipelineLayout.rootDescriptorMaxNum;

        uint32_t pipelineLayoutDescriptorSetMaxNum = NriDeref(DeviceInfo)->pipelineLayout.descriptorSetMaxNum;

        // D3D12 has limited-size root signature
        if (NriDeref(DeviceInfo)->graphicsAPI == NriScopedMember(GraphicsBackend, D3D12)) {
            const uint32_t descriptorTableCost = 4;
            const uint32_t rootDescriptorCost = 8;

            uint32_t freeBytesInRootSignature = 256;

            // Reserved 1 root descriptor for "draw parameters" emulation
            if (NriDeref(pipelineLayoutSettingsDesc)->enableD3D12DrawParametersEmulation)
                freeBytesInRootSignature -= 8;

            // Reserved 1 root constant for "draw index" emulation
            if (NriDeref(pipelineLayoutSettingsDesc)->enableD3D12DrawIndexEmulation)
                freeBytesInRootSignature -= 4;

            // Must fit
            uint32_t availableDescriptorRangeNum = freeBytesInRootSignature / descriptorTableCost;
            if (descriptorRangeNum > availableDescriptorRangeNum)
                descriptorRangeNum = availableDescriptorRangeNum;

            freeBytesInRootSignature -= descriptorRangeNum * descriptorTableCost;

            // Desired fit
            if (NriDeref(pipelineLayoutSettingsDesc)->preferRootDescriptorsOverConstants) {
                uint32_t availableRootDescriptorNum = freeBytesInRootSignature / rootDescriptorCost;
                if (rootDescriptorNum > availableRootDescriptorNum)
                    rootDescriptorNum = availableRootDescriptorNum;

                freeBytesInRootSignature -= rootDescriptorNum * rootDescriptorCost;

                if (rootConstantSize > freeBytesInRootSignature)
                    rootConstantSize = freeBytesInRootSignature;
            } else {
                if (rootConstantSize > freeBytesInRootSignature)
                    rootConstantSize = freeBytesInRootSignature;

                freeBytesInRootSignature -= rootConstantSize;

                uint32_t availableRootDescriptorNum = freeBytesInRootSignature / rootDescriptorCost;
                if (rootDescriptorNum > availableRootDescriptorNum)
                    rootDescriptorNum = availableRootDescriptorNum;
            }
        } else if (rootDescriptorNum)
            pipelineLayoutDescriptorSetMaxNum--;

        if (descriptorSetNum > pipelineLayoutDescriptorSetMaxNum)
            descriptorSetNum = pipelineLayoutDescriptorSetMaxNum;

        Nri(PipelineLayoutSettingsDesc) modifiedPipelineLayoutLimitsDesc = *NriDeref(pipelineLayoutSettingsDesc);
        modifiedPipelineLayoutLimitsDesc.descriptorSetNum = descriptorSetNum;
        modifiedPipelineLayoutLimitsDesc.descriptorRangeNum = descriptorRangeNum;
        modifiedPipelineLayoutLimitsDesc.rootConstantSize = rootConstantSize;
        modifiedPipelineLayoutLimitsDesc.rootDescriptorNum = rootDescriptorNum;

        return modifiedPipelineLayoutLimitsDesc;
    }
}