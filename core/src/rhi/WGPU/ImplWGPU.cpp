// © 2026 NVIDIA Corporation

#include <algorithm>
#include <cstdio>
#include <string>
#include <thread>

#include "SharedWGPU.h"

#include "BufferWGPU.h"
#include "CommandAllocatorWGPU.h"
#include "CommandBufferWGPU.h"
#include "DescriptorPoolWGPU.h"
#include "DescriptorSetWGPU.h"
#include "DescriptorWGPU.h"
#include "FenceWGPU.h"
#include "MemoryWGPU.h"
#include "PipelineCacheWGPU.h"
#include "PipelineLayoutWGPU.h"
#include "PipelineWGPU.h"
#include "QueryPoolWGPU.h"
#include "QueueWGPU.h"
#include "SwapChainWGPU.h"
#include "TextureWGPU.h"

#include "HelperInterface.h"
#include "ImguiInterface.h"
#include "StreamerInterface.h"
#include "UpscalerInterface.h"

using namespace Core::RHI;

#include "BufferWGPU.hpp"
#include "CommandAllocatorWGPU.hpp"
#include "CommandBufferWGPU.hpp"
#include "DescriptorPoolWGPU.hpp"
#include "DescriptorSetWGPU.hpp"
#include "DescriptorWGPU.hpp"
#include "DeviceWGPU.hpp"
#include "FenceWGPU.hpp"
#include "MemoryWGPU.hpp"
#include "PipelineCacheWGPU.hpp"
#include "PipelineLayoutWGPU.hpp"
#include "PipelineWGPU.hpp"
#include "QueryPoolWGPU.hpp"
#include "QueueWGPU.hpp"
#include "SharedWGPU.hpp"
#include "SwapChainWGPU.hpp"
#include "TextureWGPU.hpp"

Result CreateDeviceWGPU(const DeviceCreationDesc& desc, DeviceBase*& device) {
    DeviceWGPU* impl = Allocate<DeviceWGPU>(desc.allocationCallbacks, desc.callbackInterface, desc.allocationCallbacks);

    if (!impl) {
        device = nullptr;

        return Result::FAILURE;
    }

    Result result = impl->Create(desc);
    if (result != Result::Success) {
        Destroy(desc.allocationCallbacks, impl);
        device = nullptr;
    } else
        device = (DeviceBase*)impl;

    return result;
}

//============================================================================================================================================================================================
#pragma region[  Core  ]

static const DeviceInfo& GetDeviceInfo(const Device& device) {
    return ((DeviceWGPU&)device).GetDesc();
}

static const BufferDesc& GetBufferDesc(const Buffer& buffer) {
    return ((BufferWGPU&)buffer).GetDesc();
}

static const TextureDesc& GetTextureDesc(const Texture& texture) {
    return ((TextureWGPU&)texture).GetDesc();
}

static FormatSupportBits GetFormatSupport(const Device& device, Format format) {
    return ((DeviceWGPU&)device).GetFormatSupport(format);
}

static Result GetQueue(Device& device, QueueType queueType, uint32_t queueIndex, Queue*& queue) {
    return ((DeviceWGPU&)device).GetQueue(queueType, queueIndex, queue);
}

static Result CreateCommandAllocator(Queue& queue, CommandAllocator*& commandAllocator) {
    DeviceWGPU& device = ((QueueWGPU&)queue).GetDevice();
    return device.CreateImplementation<CommandAllocatorWGPU>(commandAllocator, queue);
}

static Result CreateCommandBuffer(CommandAllocator& commandAllocator, CommandBuffer*& commandBuffer) {
    return ((CommandAllocatorWGPU&)commandAllocator).CreateCommandBuffer(commandBuffer);
}

static Result CreateFence(Device& device, uint64_t initialValue, Fence*& fence) {
    return ((DeviceWGPU&)device).CreateImplementation<FenceWGPU>(fence, initialValue);
}

static Result CreateDescriptorPool(Device& device, const DescriptorPoolDesc& descriptorPoolDesc, DescriptorPool*& descriptorPool) {
    return ((DeviceWGPU&)device).CreateImplementation<DescriptorPoolWGPU>(descriptorPool, descriptorPoolDesc);
}

static Result CreatePipelineLayout(Device& device, const PipelineLayoutDesc& pipelineLayoutDesc, PipelineLayout*& pipelineLayout) {
    return ((DeviceWGPU&)device).CreateImplementation<PipelineLayoutWGPU>(pipelineLayout, pipelineLayoutDesc);
}

static Result CreateGraphicsPipeline(Device& device, const GraphicsPipelineDesc& graphicsPipelineDesc, Pipeline*& pipeline) {
    return ((DeviceWGPU&)device).CreateImplementation<PipelineWGPU>(pipeline, graphicsPipelineDesc);
}

static Result CreateComputePipeline(Device& device, const ComputePipelineDesc& computePipelineDesc, Pipeline*& pipeline) {
    return ((DeviceWGPU&)device).CreateImplementation<PipelineWGPU>(pipeline, computePipelineDesc);
}

static Result CreatePipelineCache(Device& device, const PipelineCacheDesc& pipelineCacheDesc, PipelineCache*& pipelineCache) {
    return ((DeviceWGPU&)device).CreateImplementation<PipelineCacheWGPU>(pipelineCache, pipelineCacheDesc);
}

static Result CreateQueryPool(Device& device, const QueryPoolDesc& queryPoolDesc, QueryPool*& queryPool) {
    return ((DeviceWGPU&)device).CreateImplementation<QueryPoolWGPU>(queryPool, queryPoolDesc);
}

static Result CreateSampler(Device& device, const SamplerDesc& samplerDesc, Descriptor*& sampler) {
    return ((DeviceWGPU&)device).CreateImplementation<DescriptorWGPU>(sampler, samplerDesc);
}

static Result CreateBufferView(const BufferViewDesc& bufferViewDesc, Descriptor*& bufferView) {
    DeviceWGPU& device = ((BufferWGPU*)bufferViewDesc.buffer)->GetDevice();
    return device.CreateImplementation<DescriptorWGPU>(bufferView, bufferViewDesc);
}

static Result CreateTextureView(const TextureViewDesc& textureViewDesc, Descriptor*& textureView) {
    DeviceWGPU& device = ((TextureWGPU*)textureViewDesc.texture)->GetDevice();
    return device.CreateImplementation<DescriptorWGPU>(textureView, textureViewDesc);
}

static void DestroyCommandAllocator(CommandAllocator* commandAllocator) {
    Destroy((CommandAllocatorWGPU*)commandAllocator);
}

static void DestroyCommandBuffer(CommandBuffer* commandBuffer) {
    Destroy((CommandBufferWGPU*)commandBuffer);
}

static void DestroyDescriptorPool(DescriptorPool* descriptorPool) {
    Destroy((DescriptorPoolWGPU*)descriptorPool);
}

static void DestroyBuffer(Buffer* buffer) {
    Destroy((BufferWGPU*)buffer);
}

static void DestroyTexture(Texture* texture) {
    Destroy((TextureWGPU*)texture);
}

static void DestroyDescriptor(Descriptor* descriptor) {
    Destroy((DescriptorWGPU*)descriptor);
}

static void DestroyPipelineLayout(PipelineLayout* pipelineLayout) {
    Destroy((PipelineLayoutWGPU*)pipelineLayout);
}

static void DestroyPipeline(Pipeline* pipeline) {
    Destroy((PipelineWGPU*)pipeline);
}

static void DestroyPipelineCache(PipelineCache* pipelineCache) {
    Destroy((PipelineCacheWGPU*)pipelineCache);
}

static Result GetPipelineCacheData(PipelineCache& pipelineCache, void* dst, uint64_t& size) {
    return ((PipelineCacheWGPU&)pipelineCache).GetData(dst, size);
}

static void DestroyQueryPool(QueryPool* queryPool) {
    Destroy((QueryPoolWGPU*)queryPool);
}

static void DestroyFence(Fence* fence) {
    Destroy((FenceWGPU*)fence);
}

static Result AllocateMemory(Device& device, const AllocateMemoryDesc& allocateMemoryDesc, Memory*& memory) {
    return ((DeviceWGPU&)device).CreateImplementation<MemoryWGPU>(memory, allocateMemoryDesc);
}

static void FreeMemory(Memory* memory) {
    Destroy((MemoryWGPU*)memory);
}

static Result CreateBuffer(Device& device, const BufferDesc& bufferDesc, Buffer*& buffer) {
    return ((DeviceWGPU&)device).CreateImplementation<BufferWGPU>(buffer, bufferDesc);
}

static Result CreateTexture(Device& device, const TextureDesc& textureDesc, Texture*& texture) {
    return ((DeviceWGPU&)device).CreateImplementation<TextureWGPU>(texture, textureDesc);
}

static void GetBufferMemoryDesc(const Buffer& buffer, MemoryLocation memoryLocation, MemoryDesc& memoryDesc) {
    const BufferDesc& bufferDesc = ((BufferWGPU&)buffer).GetDesc();
    memoryDesc = {std::max(bufferDesc.size, 1ull), 1, (MemoryType)memoryLocation, false};
}

static void GetTextureMemoryDesc(const Texture& texture, MemoryLocation memoryLocation, MemoryDesc& memoryDesc) {
    const TextureDesc& textureDesc = ((TextureWGPU&)texture).GetDesc();
    uint64_t size = (uint64_t)textureDesc.width * std::max((Dim_t)1, textureDesc.height) * std::max((Dim_t)1, textureDesc.depth) * std::max((Dim_t)1, textureDesc.layerNum) * std::max((Dim_t)1, textureDesc.mipNum);

    memoryDesc = {std::max(size, 1ull), 1, (MemoryType)memoryLocation, false};
}

static Result BindBufferMemory(const BindBufferMemoryDesc* bindBufferMemoryDescs, uint32_t bindBufferMemoryDescNum) {
    for (uint32_t i = 0; i < bindBufferMemoryDescNum; i++) {
        BufferWGPU& buffer = *(BufferWGPU*)bindBufferMemoryDescs[i].buffer;
        const MemoryWGPU& memory = *(MemoryWGPU*)bindBufferMemoryDescs[i].memory;
        Result result = buffer.SetHostVisible(memory.GetMemoryLocation());
        if (result != Result::Success)
            return result;
    }

    return Result::Success;
}

static Result BindTextureMemory(const BindTextureMemoryDesc*, uint32_t) {
    return Result::Success;
}

static void GetBufferMemoryDesc2(const Device&, const BufferDesc& bufferDesc, MemoryLocation memoryLocation, MemoryDesc& memoryDesc) {
    memoryDesc = {std::max(bufferDesc.size, 1ull), 1, (MemoryType)memoryLocation, false};
}

static void GetTextureMemoryDesc2(const Device&, const TextureDesc& textureDesc, MemoryLocation memoryLocation, MemoryDesc& memoryDesc) {
    uint64_t size = (uint64_t)textureDesc.width * std::max((Dim_t)1, textureDesc.height) * std::max((Dim_t)1, textureDesc.depth) * std::max((Dim_t)1, textureDesc.layerNum) * std::max((Dim_t)1, textureDesc.mipNum);

    memoryDesc = {std::max(size, 1ull), 1, (MemoryType)memoryLocation, false};
}

static Result CreateCommittedBuffer(Device& device, MemoryLocation memoryLocation, float priority, const BufferDesc& bufferDesc, Buffer*& buffer) {
    MaybeUnused(priority);

    return ((DeviceWGPU&)device).CreateImplementation<BufferWGPU>(buffer, bufferDesc, memoryLocation);
}

static Result CreateCommittedTexture(Device& device, MemoryLocation memoryLocation, float priority, const TextureDesc& textureDesc, Texture*& texture) {
    MaybeUnused(memoryLocation, priority);

    return ((DeviceWGPU&)device).CreateImplementation<TextureWGPU>(texture, textureDesc);
}

static Result CreatePlacedBuffer(Device& device, Memory* memory, uint64_t offset, const BufferDesc& bufferDesc, Buffer*& buffer) {
    MaybeUnused(offset);

    MemoryLocation memoryLocation = memory ? ((MemoryWGPU*)memory)->GetMemoryLocation() : MemoryLocation::DEVICE;

    return ((DeviceWGPU&)device).CreateImplementation<BufferWGPU>(buffer, bufferDesc, memoryLocation);
}

static Result CreatePlacedTexture(Device& device, Memory* memory, uint64_t offset, const TextureDesc& textureDesc, Texture*& texture) {
    MaybeUnused(memory, offset);

    return ((DeviceWGPU&)device).CreateImplementation<TextureWGPU>(texture, textureDesc);
}

static Result AllocateDescriptorSets(DescriptorPool& descriptorPool, const PipelineLayout& pipelineLayout, uint32_t setIndex, DescriptorSet** descriptorSets, uint32_t instanceNum, uint32_t variableDescriptorNum) {
    return ((DescriptorPoolWGPU&)descriptorPool).AllocateDescriptorSets(pipelineLayout, setIndex, descriptorSets, instanceNum, variableDescriptorNum);
}

static void AddTouchedDescriptorSet(DescriptorSetWGPU** touchedSets, uint32_t& touchedSetNum, DescriptorSetWGPU* descriptorSet) {
    for (uint32_t i = 0; i < touchedSetNum; i++) {
        if (touchedSets[i] == descriptorSet)
            return;
    }

    touchedSets[touchedSetNum++] = descriptorSet;
}

static void UpdateDescriptorRanges(const UpdateDescriptorRangeDesc* updateDescriptorRangeDescs, uint32_t updateDescriptorRangeDescNum) {
    if (!updateDescriptorRangeDescNum)
        return;

    DeviceWGPU& device = ((DescriptorSetWGPU*)updateDescriptorRangeDescs[0].descriptorSet)->GetDevice();
    Scratch<DescriptorSetWGPU*> touchedSets = NRI_ALLOCATE_SCRATCH(device, DescriptorSetWGPU*, updateDescriptorRangeDescNum);
    uint32_t touchedSetNum = 0;

    for (uint32_t i = 0; i < updateDescriptorRangeDescNum; i++) {
        const UpdateDescriptorRangeDesc& desc = updateDescriptorRangeDescs[i];
        DescriptorSetWGPU* descriptorSet = (DescriptorSetWGPU*)desc.descriptorSet;
        descriptorSet->UpdateRange(desc.rangeIndex, desc.baseDescriptor, desc.descriptors, desc.descriptorNum);

        AddTouchedDescriptorSet(touchedSets, touchedSetNum, descriptorSet);
    }

    for (uint32_t i = 0; i < touchedSetNum; i++)
        touchedSets[i]->FinalizeUpdate();
}

static void CopyDescriptorRanges(const CopyDescriptorRangeDesc* copyDescriptorRangeDescs, uint32_t copyDescriptorRangeDescNum) {
    if (!copyDescriptorRangeDescNum)
        return;

    DeviceWGPU& device = ((DescriptorSetWGPU*)copyDescriptorRangeDescs[0].dstDescriptorSet)->GetDevice();
    Scratch<DescriptorSetWGPU*> touchedSets = NRI_ALLOCATE_SCRATCH(device, DescriptorSetWGPU*, copyDescriptorRangeDescNum);
    uint32_t touchedSetNum = 0;

    for (uint32_t i = 0; i < copyDescriptorRangeDescNum; i++) {
        const CopyDescriptorRangeDesc& desc = copyDescriptorRangeDescs[i];
        DescriptorSetWGPU* descriptorSet = (DescriptorSetWGPU*)desc.dstDescriptorSet;
        descriptorSet->CopyRangeFrom(desc.dstRangeIndex, desc.dstBaseDescriptor, *(DescriptorSetWGPU*)desc.srcDescriptorSet, desc.srcRangeIndex, desc.srcBaseDescriptor, desc.descriptorNum);

        AddTouchedDescriptorSet(touchedSets, touchedSetNum, descriptorSet);
    }

    for (uint32_t i = 0; i < touchedSetNum; i++)
        touchedSets[i]->FinalizeUpdate();
}

static void ResetDescriptorPool(DescriptorPool& descriptorPool) {
    ((DescriptorPoolWGPU&)descriptorPool).Reset();
}

static void GetDescriptorSetOffsets(const DescriptorSet& descriptorSet, uint32_t& resourceHeapOffset, uint32_t& samplerHeapOffset) {
    ((DescriptorSetWGPU&)descriptorSet).GetOffsets(resourceHeapOffset, samplerHeapOffset);
}

static Result BeginCommandBuffer(CommandBuffer& commandBuffer, const DescriptorPool* descriptorPool) {
    return ((CommandBufferWGPU&)commandBuffer).Begin(descriptorPool);
}

static void CmdSetDescriptorPool(CommandBuffer&, const DescriptorPool&) {
}

static void CmdSetPipelineLayout(CommandBuffer& commandBuffer, BindPoint bindPoint, const PipelineLayout& pipelineLayout) {
    ((CommandBufferWGPU&)commandBuffer).SetPipelineLayout(bindPoint, pipelineLayout);
}

static void CmdSetDescriptorSet(CommandBuffer& commandBuffer, const SetDescriptorSetDesc& setDescriptorSetDesc) {
    ((CommandBufferWGPU&)commandBuffer).SetDescriptorSet(setDescriptorSetDesc);
}

static void CmdSetRootConstants(CommandBuffer& commandBuffer, const SetRootConstantsDesc& setRootConstantsDesc) {
    ((CommandBufferWGPU&)commandBuffer).SetRootConstants(setRootConstantsDesc);
}

static void CmdSetRootDescriptor(CommandBuffer& commandBuffer, const SetRootDescriptorDesc& setRootDescriptorDesc) {
    ((CommandBufferWGPU&)commandBuffer).SetRootDescriptor(setRootDescriptorDesc);
}

static void CmdSetPipeline(CommandBuffer& commandBuffer, const Pipeline& pipeline) {
    ((CommandBufferWGPU&)commandBuffer).SetPipeline(pipeline);
}

static void CmdBarrier(CommandBuffer& commandBuffer, const BarrierDesc& barrierDesc) {
    ((CommandBufferWGPU&)commandBuffer).Barrier(barrierDesc);
}

static void CmdSetIndexBuffer(CommandBuffer& commandBuffer, const Buffer& buffer, uint64_t offset, IndexType indexType) {
    ((CommandBufferWGPU&)commandBuffer).SetIndexBuffer(buffer, offset, indexType);
}

static void CmdSetVertexBuffers(CommandBuffer& commandBuffer, uint32_t baseSlot, const VertexBufferDesc* vertexBufferDescs, uint32_t vertexBufferNum) {
    ((CommandBufferWGPU&)commandBuffer).SetVertexBuffers(baseSlot, vertexBufferDescs, vertexBufferNum);
}

static void CmdSetViewports(CommandBuffer& commandBuffer, const Viewport* viewports, uint32_t viewportNum) {
    ((CommandBufferWGPU&)commandBuffer).SetViewports(viewports, viewportNum);
}

static void CmdSetScissors(CommandBuffer& commandBuffer, const Rect* rects, uint32_t rectNum) {
    ((CommandBufferWGPU&)commandBuffer).SetScissors(rects, rectNum);
}

static void CmdSetStencilReference(CommandBuffer& commandBuffer, uint8_t frontRef, uint8_t backRef) {
    ((CommandBufferWGPU&)commandBuffer).SetStencilReference(frontRef, backRef);
}

static void CmdSetDepthBounds(CommandBuffer&, float, float) {
    // TODO: WebGPU does not support dynamic depth bounds. Keep "features.depthBoundsTest = false".
}

static void CmdSetBlendConstants(CommandBuffer& commandBuffer, const Color32f& color) {
    ((CommandBufferWGPU&)commandBuffer).SetBlendConstants(color);
}

static void CmdSetSampleLocations(CommandBuffer&, const SampleLocation*, Sample_t, Sample_t) {
    // TODO: WebGPU does not support programmable sample locations. Keep "tiers.sampleLocations = 0".
}

static void CmdSetShadingRate(CommandBuffer&, const ShadingRateDesc&) {
    // TODO: WebGPU does not support variable rate shading. Keep "tiers.shadingRate = 0".
}

static void CmdSetDepthBias(CommandBuffer&, const DepthBiasDesc&) {
    // TODO: WebGPU supports pipeline depth bias only, not dynamic depth bias. Keep "features.dynamicDepthBias = false".
}

static void CmdBeginRendering(CommandBuffer& commandBuffer, const RenderingDesc& renderingDesc) {
    ((CommandBufferWGPU&)commandBuffer).BeginRendering(renderingDesc);
}

static void CmdClearAttachments(CommandBuffer& commandBuffer, const ClearAttachmentDesc* clearAttachmentDescs, uint32_t clearAttachmentDescNum, const Rect* rects, uint32_t rectNum) {
    ((CommandBufferWGPU&)commandBuffer).ClearAttachments(clearAttachmentDescs, clearAttachmentDescNum, rects, rectNum);
}

static void CmdDraw(CommandBuffer& commandBuffer, const DrawDesc& drawDesc) {
    ((CommandBufferWGPU&)commandBuffer).Draw(drawDesc);
}

static void CmdDrawIndexed(CommandBuffer& commandBuffer, const DrawIndexedDesc& drawIndexedDesc) {
    ((CommandBufferWGPU&)commandBuffer).DrawIndexed(drawIndexedDesc);
}

static void CmdDrawIndirect(CommandBuffer& commandBuffer, const Buffer& buffer, uint64_t offset, uint32_t drawNum, uint32_t stride, const Buffer* countBuffer, uint64_t countBufferOffset) {
    ((CommandBufferWGPU&)commandBuffer).DrawIndirect(buffer, offset, drawNum, stride, countBuffer, countBufferOffset);
}

static void CmdDrawIndexedIndirect(CommandBuffer& commandBuffer, const Buffer& buffer, uint64_t offset, uint32_t drawNum, uint32_t stride, const Buffer* countBuffer, uint64_t countBufferOffset) {
    ((CommandBufferWGPU&)commandBuffer).DrawIndexedIndirect(buffer, offset, drawNum, stride, countBuffer, countBufferOffset);
}

static void CmdEndRendering(CommandBuffer& commandBuffer) {
    ((CommandBufferWGPU&)commandBuffer).EndRendering();
}

static void CmdDispatch(CommandBuffer& commandBuffer, const DispatchDesc& dispatchDesc) {
    ((CommandBufferWGPU&)commandBuffer).Dispatch(dispatchDesc);
}

static void CmdDispatchIndirect(CommandBuffer& commandBuffer, const Buffer& buffer, uint64_t offset) {
    ((CommandBufferWGPU&)commandBuffer).DispatchIndirect(buffer, offset);
}

static void CmdCopyBuffer(CommandBuffer& commandBuffer, Buffer& dstBuffer, uint64_t dstOffset, const Buffer& srcBuffer, uint64_t srcOffset, uint64_t size) {
    ((CommandBufferWGPU&)commandBuffer).CopyBuffer(dstBuffer, dstOffset, srcBuffer, srcOffset, size);
}

static void CmdCopyTexture(CommandBuffer& commandBuffer, Texture& dstTexture, const TextureRegionDesc* dstRegion, const Texture& srcTexture, const TextureRegionDesc* srcRegion) {
    ((CommandBufferWGPU&)commandBuffer).CopyTexture(dstTexture, dstRegion, srcTexture, srcRegion);
}

static void CmdUploadBufferToTexture(CommandBuffer& commandBuffer, Texture& dstTexture, const TextureRegionDesc& dstRegion, const Buffer& srcBuffer, const TextureDataLayoutDesc& srcDataLayout) {
    ((CommandBufferWGPU&)commandBuffer).UploadBufferToTexture(dstTexture, dstRegion, srcBuffer, srcDataLayout);
}

static void CmdReadbackTextureToBuffer(CommandBuffer& commandBuffer, Buffer& dstBuffer, const TextureDataLayoutDesc& dstDataLayout, const Texture& srcTexture, const TextureRegionDesc& srcRegion) {
    ((CommandBufferWGPU&)commandBuffer).ReadbackTextureToBuffer(dstBuffer, dstDataLayout, srcTexture, srcRegion);
}

static void CmdZeroBuffer(CommandBuffer& commandBuffer, Buffer& buffer, uint64_t offset, uint64_t size) {
    ((CommandBufferWGPU&)commandBuffer).ZeroBuffer(buffer, offset, size);
}

static void CmdResolveTexture(CommandBuffer& commandBuffer, Texture& dstTexture, const TextureRegionDesc* dstRegion, const Texture& srcTexture, const TextureRegionDesc* srcRegion, ResolveOp resolveOp) {
    ((CommandBufferWGPU&)commandBuffer).ResolveTexture(dstTexture, dstRegion, srcTexture, srcRegion, resolveOp);
}

static void CmdClearStorage(CommandBuffer& commandBuffer, const ClearStorageDesc& clearStorageDesc) {
    ((CommandBufferWGPU&)commandBuffer).ClearStorage(clearStorageDesc);
}

static void CmdResetQueries(CommandBuffer& commandBuffer, QueryPool& queryPool, uint32_t offset, uint32_t num) {
    ((CommandBufferWGPU&)commandBuffer).ResetQueries(queryPool, offset, num);
}

static void CmdBeginQuery(CommandBuffer& commandBuffer, QueryPool& queryPool, uint32_t offset) {
    ((CommandBufferWGPU&)commandBuffer).BeginQuery(queryPool, offset);
}

static void CmdEndQuery(CommandBuffer& commandBuffer, QueryPool& queryPool, uint32_t offset) {
    ((CommandBufferWGPU&)commandBuffer).EndQuery(queryPool, offset);
}

static void CmdCopyQueries(CommandBuffer& commandBuffer, const QueryPool& queryPool, uint32_t offset, uint32_t num, Buffer& dstBuffer, uint64_t dstOffset) {
    ((CommandBufferWGPU&)commandBuffer).CopyQueries(queryPool, offset, num, dstBuffer, dstOffset);
}

static void CmdBeginAnnotation(CommandBuffer& commandBuffer, const char* name, uint32_t bgra) {
    ((CommandBufferWGPU&)commandBuffer).BeginAnnotation(name, bgra);
}

static void CmdEndAnnotation(CommandBuffer& commandBuffer) {
    ((CommandBufferWGPU&)commandBuffer).EndAnnotation();
}

static void CmdAnnotation(CommandBuffer& commandBuffer, const char* name, uint32_t bgra) {
    ((CommandBufferWGPU&)commandBuffer).Annotation(name, bgra);
}

static Result EndCommandBuffer(CommandBuffer& commandBuffer) {
    return ((CommandBufferWGPU&)commandBuffer).End();
}

static void QueueBeginAnnotation(Queue& queue, const char* name, uint32_t bgra) {
    ((QueueWGPU&)queue).BeginAnnotation(name, bgra);
}

static void QueueEndAnnotation(Queue& queue) {
    ((QueueWGPU&)queue).EndAnnotation();
}

static void QueueAnnotation(Queue& queue, const char* name, uint32_t bgra) {
    ((QueueWGPU&)queue).Annotation(name, bgra);
}

static void GetCalibratedTimestamps(Queue& queue, uint64_t& timestampGPU, uint64_t& timestampCPU) {
    ((QueueWGPU&)queue).GetCalibratedTimestamps(timestampGPU, timestampCPU);
}

static void ResetQueries(QueryPool& queryPool, uint32_t offset, uint32_t num) {
    ((QueryPoolWGPU&)queryPool).Reset(offset, num);
}

static uint32_t GetQuerySize(const QueryPool& queryPool) {
    return ((QueryPoolWGPU&)queryPool).GetQuerySize();
}

static Result QueueSubmit(Queue& queue, const QueueSubmitDesc& queueSubmitDesc) {
    return ((QueueWGPU&)queue).Submit(queueSubmitDesc);
}

static Result DeviceWaitIdle(Device* device) {
    return ((DeviceWGPU*)device)->WaitIdle();
}

static Result QueueWaitIdle(Queue* queue) {
    return ((QueueWGPU*)queue)->WaitIdle();
}

static void Wait(Fence& fence, uint64_t value) {
    ((FenceWGPU&)fence).Wait(value);
}

static uint64_t GetFenceValue(Fence& fence) {
    return ((FenceWGPU&)fence).GetValue();
}

static void ResetCommandAllocator(CommandAllocator& commandAllocator) {
    ((CommandAllocatorWGPU&)commandAllocator).Reset();
}

static void* MapBuffer(Buffer& buffer, uint64_t offset, uint64_t size) {
    return ((BufferWGPU&)buffer).Map(offset, size);
}

static void UnmapBuffer(Buffer& buffer) {
    ((BufferWGPU&)buffer).Unmap();
}

static uint64_t GetBufferDeviceAddress(const Buffer&) {
    // TODO: WebGPU does not expose buffer device addresses. Keep device-address/ray-tracing features disabled.
    return 0;
}

static void SetDebugName(Object* object, const char* name) {
    MaybeUnused(object, name);
#if NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS
    if (object)
        ((DebugNameBase*)object)->SetDebugName(name);
#endif
}

static void* GetDeviceNativeObject(const Device* device) {
    if (!device)
        return nullptr;

    return (WGPUDevice)(*(DeviceWGPU*)device);
}

static void* GetQueueNativeObject(const Queue* queue) {
    if (!queue)
        return nullptr;

    return ((QueueWGPU*)queue)->GetDevice().GetQueue();
}

static void* GetCommandBufferNativeObject(const CommandBuffer* commandBuffer) {
    if (!commandBuffer)
        return nullptr;

    return ((CommandBufferWGPU*)commandBuffer)->GetCommandBuffer();
}

static uint64_t GetBufferNativeObject(const Buffer* buffer) {
    if (!buffer)
        return 0;

    return uint64_t((WGPUBuffer)(*(BufferWGPU*)buffer));
}

static uint64_t GetTextureNativeObject(const Texture* texture) {
    if (!texture)
        return 0;

    return uint64_t((WGPUTexture)(*(TextureWGPU*)texture));
}

static uint64_t GetDescriptorNativeObject(const Descriptor* descriptor) {
    if (!descriptor)
        return 0;

    DescriptorWGPU& descriptorWGPU = *(DescriptorWGPU*)descriptor;
    switch (descriptorWGPU.GetDescriptorType()) {
        case DescriptorType::SAMPLER:
            return uint64_t(descriptorWGPU.GetSampler());
        case DescriptorType::TEXTURE:
        case DescriptorType::STORAGE_TEXTURE:
        case DescriptorType::MUTABLE:
        case DescriptorType::INPUT_ATTACHMENT:
            return uint64_t(descriptorWGPU.GetTextureView());
        default:
            return uint64_t(descriptorWGPU.GetBuffer());
    }
}

Result DeviceWGPU::FillFunctionTable(CoreInterface& table) const {
    table.GetDeviceInfo = ::GetDeviceInfo;
    table.GetBufferDesc = ::GetBufferDesc;
    table.GetTextureDesc = ::GetTextureDesc;
    table.GetFormatSupport = ::GetFormatSupport;
    table.GetQuerySize = ::GetQuerySize;
    table.GetFenceValue = ::GetFenceValue;
    table.GetDescriptorSetOffsets = ::GetDescriptorSetOffsets;
    table.GetQueue = ::GetQueue;
    table.CreateCommandAllocator = ::CreateCommandAllocator;
    table.CreateCommandBuffer = ::CreateCommandBuffer;
    table.CreateDescriptorPool = ::CreateDescriptorPool;
    table.CreateBufferView = ::CreateBufferView;
    table.CreateTextureView = ::CreateTextureView;
    table.CreateSampler = ::CreateSampler;
    table.CreatePipelineLayout = ::CreatePipelineLayout;
    table.CreateGraphicsPipeline = ::CreateGraphicsPipeline;
    table.CreateComputePipeline = ::CreateComputePipeline;
    table.CreatePipelineCache = ::CreatePipelineCache;
    table.CreateQueryPool = ::CreateQueryPool;
    table.CreateFence = ::CreateFence;
    table.DestroyCommandAllocator = ::DestroyCommandAllocator;
    table.DestroyCommandBuffer = ::DestroyCommandBuffer;
    table.DestroyDescriptorPool = ::DestroyDescriptorPool;
    table.DestroyBuffer = ::DestroyBuffer;
    table.DestroyTexture = ::DestroyTexture;
    table.DestroyDescriptor = ::DestroyDescriptor;
    table.DestroyPipelineLayout = ::DestroyPipelineLayout;
    table.DestroyPipeline = ::DestroyPipeline;
    table.DestroyPipelineCache = ::DestroyPipelineCache;
    table.GetPipelineCacheData = ::GetPipelineCacheData;
    table.DestroyQueryPool = ::DestroyQueryPool;
    table.DestroyFence = ::DestroyFence;
    table.AllocateMemory = ::AllocateMemory;
    table.FreeMemory = ::FreeMemory;
    table.CreateBuffer = ::CreateBuffer;
    table.CreateTexture = ::CreateTexture;
    table.GetBufferMemoryDesc = ::GetBufferMemoryDesc;
    table.GetTextureMemoryDesc = ::GetTextureMemoryDesc;
    table.BindBufferMemory = ::BindBufferMemory;
    table.BindTextureMemory = ::BindTextureMemory;
    table.GetBufferMemoryDesc2 = ::GetBufferMemoryDesc2;
    table.GetTextureMemoryDesc2 = ::GetTextureMemoryDesc2;
    table.CreateCommittedBuffer = ::CreateCommittedBuffer;
    table.CreateCommittedTexture = ::CreateCommittedTexture;
    table.CreatePlacedBuffer = ::CreatePlacedBuffer;
    table.CreatePlacedTexture = ::CreatePlacedTexture;
    table.AllocateDescriptorSets = ::AllocateDescriptorSets;
    table.UpdateDescriptorRanges = ::UpdateDescriptorRanges;
    table.CopyDescriptorRanges = ::CopyDescriptorRanges;
    table.ResetDescriptorPool = ::ResetDescriptorPool;
    table.BeginCommandBuffer = ::BeginCommandBuffer;
    table.CmdSetDescriptorPool = ::CmdSetDescriptorPool;
    table.CmdSetDescriptorSet = ::CmdSetDescriptorSet;
    table.CmdSetPipelineLayout = ::CmdSetPipelineLayout;
    table.CmdSetPipeline = ::CmdSetPipeline;
    table.CmdSetRootConstants = ::CmdSetRootConstants;
    table.CmdSetRootDescriptor = ::CmdSetRootDescriptor;
    table.CmdBarrier = ::CmdBarrier;
    table.CmdSetIndexBuffer = ::CmdSetIndexBuffer;
    table.CmdSetVertexBuffers = ::CmdSetVertexBuffers;
    table.CmdSetViewports = ::CmdSetViewports;
    table.CmdSetScissors = ::CmdSetScissors;
    table.CmdSetStencilReference = ::CmdSetStencilReference;
    table.CmdSetDepthBounds = ::CmdSetDepthBounds;
    table.CmdSetBlendConstants = ::CmdSetBlendConstants;
    table.CmdSetSampleLocations = ::CmdSetSampleLocations;
    table.CmdSetShadingRate = ::CmdSetShadingRate;
    table.CmdSetDepthBias = ::CmdSetDepthBias;
    table.CmdBeginRendering = ::CmdBeginRendering;
    table.CmdClearAttachments = ::CmdClearAttachments;
    table.CmdDraw = ::CmdDraw;
    table.CmdDrawIndexed = ::CmdDrawIndexed;
    table.CmdDrawIndirect = ::CmdDrawIndirect;
    table.CmdDrawIndexedIndirect = ::CmdDrawIndexedIndirect;
    table.CmdEndRendering = ::CmdEndRendering;
    table.CmdDispatch = ::CmdDispatch;
    table.CmdDispatchIndirect = ::CmdDispatchIndirect;
    table.CmdCopyBuffer = ::CmdCopyBuffer;
    table.CmdCopyTexture = ::CmdCopyTexture;
    table.CmdUploadBufferToTexture = ::CmdUploadBufferToTexture;
    table.CmdReadbackTextureToBuffer = ::CmdReadbackTextureToBuffer;
    table.CmdZeroBuffer = ::CmdZeroBuffer;
    table.CmdResolveTexture = ::CmdResolveTexture;
    table.CmdClearStorage = ::CmdClearStorage;
    table.CmdResetQueries = ::CmdResetQueries;
    table.CmdBeginQuery = ::CmdBeginQuery;
    table.CmdEndQuery = ::CmdEndQuery;
    table.CmdCopyQueries = ::CmdCopyQueries;
    table.CmdBeginAnnotation = ::CmdBeginAnnotation;
    table.CmdEndAnnotation = ::CmdEndAnnotation;
    table.CmdAnnotation = ::CmdAnnotation;
    table.EndCommandBuffer = ::EndCommandBuffer;
    table.QueueBeginAnnotation = ::QueueBeginAnnotation;
    table.QueueEndAnnotation = ::QueueEndAnnotation;
    table.QueueAnnotation = ::QueueAnnotation;
    table.GetCalibratedTimestamps = ::GetCalibratedTimestamps;
    table.ResetQueries = ::ResetQueries;
    table.QueueSubmit = ::QueueSubmit;
    table.QueueWaitIdle = ::QueueWaitIdle;
    table.DeviceWaitIdle = ::DeviceWaitIdle;
    table.Wait = ::Wait;
    table.ResetCommandAllocator = ::ResetCommandAllocator;
    table.MapBuffer = ::MapBuffer;
    table.UnmapBuffer = ::UnmapBuffer;
    table.GetBufferDeviceAddress = ::GetBufferDeviceAddress;
    table.SetDebugName = ::SetDebugName;
    table.GetDeviceNativeObject = ::GetDeviceNativeObject;
    table.GetQueueNativeObject = ::GetQueueNativeObject;
    table.GetCommandBufferNativeObject = ::GetCommandBufferNativeObject;
    table.GetBufferNativeObject = ::GetBufferNativeObject;
    table.GetTextureNativeObject = ::GetTextureNativeObject;
    table.GetDescriptorNativeObject = ::GetDescriptorNativeObject;

    return Result::Success;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  Helper  ]

static Result UploadData(Queue& queue, const TextureUploadDesc* textureUploadDescs, uint32_t textureUploadDescNum, const BufferUploadDesc* bufferUploadDescs, uint32_t bufferUploadDescNum) {
    QueueWGPU& queueWGPU = (QueueWGPU&)queue;
    DeviceWGPU& deviceWGPU = queueWGPU.GetDevice();
    HelperDataUpload helperDataUpload(deviceWGPU.GetCoreInterface(), (Device&)deviceWGPU, queue);

    return helperDataUpload.UploadData(textureUploadDescs, textureUploadDescNum, bufferUploadDescs, bufferUploadDescNum);
}

static uint32_t CalculateAllocationNumber(const Device& device, const ResourceGroupDesc& resourceGroupDesc) {
    DeviceWGPU& deviceWGPU = (DeviceWGPU&)device;
    HelperDeviceMemoryAllocator allocator(deviceWGPU.GetCoreInterface(), (Device&)device);

    return allocator.CalculateAllocationNumber(resourceGroupDesc);
}

static Result AllocateAndBindMemory(Device& device, const ResourceGroupDesc& resourceGroupDesc, Memory** allocations) {
    DeviceWGPU& deviceWGPU = (DeviceWGPU&)device;
    HelperDeviceMemoryAllocator allocator(deviceWGPU.GetCoreInterface(), device);

    return allocator.AllocateAndBindMemory(resourceGroupDesc, allocations);
}

static Result QueryVideoMemoryInfo(const Device&, MemoryLocation, VideoMemoryInfo& videoMemoryInfo) {
    videoMemoryInfo = {};

    return Result::Success;
}

Result DeviceWGPU::FillFunctionTable(HelperInterface& table) const {
    table.CalculateAllocationNumber = ::CalculateAllocationNumber;
    table.AllocateAndBindMemory = ::AllocateAndBindMemory;
    table.UploadData = ::UploadData;
    table.QueryVideoMemoryInfo = ::QueryVideoMemoryInfo;

    return Result::Success;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  Imgui  ]

#if NRI_ENABLE_IMGUI_EXTENSION

static Result CreateImgui(Device& device, const ImguiDesc& imguiDesc, Imgui*& imgui) {
    DeviceWGPU& deviceWGPU = (DeviceWGPU&)device;
    ImguiImpl* impl = Allocate<ImguiImpl>(deviceWGPU.GetAllocationCallbacks(), device, deviceWGPU.GetCoreInterface());
    if (!impl)
        return Result::OUT_OF_MEMORY;

    Result result = impl->Create(imguiDesc);

    if (result != Result::Success) {
        Destroy(impl);
        imgui = nullptr;
    } else
        imgui = (Imgui*)impl;

    return result;
}

static void DestroyImgui(Imgui* imgui) {
    Destroy((ImguiImpl*)imgui);
}

static void CmdCopyImguiData(CommandBuffer& commandBuffer, Streamer& streamer, Imgui& imgui, const CopyImguiDataDesc& copyImguiDataDesc) {
    ImguiImpl& imguiImpl = (ImguiImpl&)imgui;

    return imguiImpl.CmdCopyData(commandBuffer, streamer, copyImguiDataDesc);
}

static void CmdDrawImgui(CommandBuffer& commandBuffer, Imgui& imgui, const DrawImguiDesc& drawImguiDesc) {
    ImguiImpl& imguiImpl = (ImguiImpl&)imgui;

    return imguiImpl.CmdDraw(commandBuffer, drawImguiDesc);
}

Result DeviceWGPU::FillFunctionTable(ImguiInterface& table) const {
    table.CreateImgui = ::CreateImgui;
    table.DestroyImgui = ::DestroyImgui;
    table.CmdCopyImguiData = ::CmdCopyImguiData;
    table.CmdDrawImgui = ::CmdDrawImgui;

    return Result::Success;
}

#endif

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  LowLatency  ]

Result DeviceWGPU::FillFunctionTable(LowLatencyInterface&) const {
    // TODO: WebGPU has no NRI low-latency extension mapping yet. Keep "features.lowLatency = false".
    return Result::UNSUPPORTED;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  MeshShader  ]

Result DeviceWGPU::FillFunctionTable(MeshShaderInterface&) const {
    // TODO: WebGPU does not expose mesh shaders. Keep "features.meshShader = false".
    return Result::UNSUPPORTED;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  RayTracing  ]

Result DeviceWGPU::FillFunctionTable(RayTracingInterface&) const {
    // TODO: WebGPU does not expose ray tracing or acceleration structures.
    return Result::UNSUPPORTED;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  Streamer  ]

static Result CreateStreamer(Device& device, const StreamerDesc& streamerDesc, Streamer*& streamer) {
    DeviceWGPU& deviceWGPU = (DeviceWGPU&)device;
    StreamerImpl* impl = Allocate<StreamerImpl>(deviceWGPU.GetAllocationCallbacks(), device, deviceWGPU.GetCoreInterface());
    if (!impl)
        return Result::OUT_OF_MEMORY;

    Result result = impl->Create(streamerDesc);

    if (result != Result::Success) {
        Destroy(impl);
        streamer = nullptr;
    } else
        streamer = (Streamer*)impl;

    return result;
}

static void DestroyStreamer(Streamer* streamer) {
    Destroy((StreamerImpl*)streamer);
}

static Buffer* GetStreamerConstantBuffer(Streamer& streamer) {
    return ((StreamerImpl&)streamer).GetConstantBuffer();
}

static uint32_t StreamConstantData(Streamer& streamer, const void* data, uint32_t dataSize) {
    return ((StreamerImpl&)streamer).StreamConstantData(data, dataSize);
}

static BufferOffset StreamBufferData(Streamer& streamer, const StreamBufferDataDesc& streamBufferDataDesc) {
    return ((StreamerImpl&)streamer).StreamBufferData(streamBufferDataDesc);
}

static BufferOffset StreamTextureData(Streamer& streamer, const StreamTextureDataDesc& streamTextureDataDesc) {
    return ((StreamerImpl&)streamer).StreamTextureData(streamTextureDataDesc);
}

static void EndStreamerFrame(Streamer& streamer) {
    ((StreamerImpl&)streamer).EndFrame();
}

static void CmdCopyStreamedData(CommandBuffer& commandBuffer, Streamer& streamer) {
    ((StreamerImpl&)streamer).CmdCopyStreamedData(commandBuffer);
}

Result DeviceWGPU::FillFunctionTable(StreamerInterface& table) const {
    table.CreateStreamer = ::CreateStreamer;
    table.DestroyStreamer = ::DestroyStreamer;
    table.GetStreamerConstantBuffer = ::GetStreamerConstantBuffer;
    table.StreamBufferData = ::StreamBufferData;
    table.StreamTextureData = ::StreamTextureData;
    table.StreamConstantData = ::StreamConstantData;
    table.EndStreamerFrame = ::EndStreamerFrame;
    table.CmdCopyStreamedData = ::CmdCopyStreamedData;

    return Result::Success;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  SwapChain  ]

static Result CreateSwapChain(Device& device, const SwapChainDesc& swapChainDesc, SwapChain*& swapChain) {
    return ((DeviceWGPU&)device).CreateImplementation<SwapChainWGPU>(swapChain, swapChainDesc);
}

static void DestroySwapChain(SwapChain* swapChain) {
    Destroy((SwapChainWGPU*)swapChain);
}

static Texture* const* GetSwapChainTextures(const SwapChain& swapChain, uint32_t& textureNum) {
    return ((SwapChainWGPU&)swapChain).GetTextures(textureNum);
}

static Result GetDisplayDesc(SwapChain& swapChain, DisplayDesc& displayDesc) {
    return ((SwapChainWGPU&)swapChain).GetDisplayDesc(displayDesc);
}

static Result AcquireNextTexture(SwapChain& swapChain, Fence&, uint32_t& textureIndex) {
    return ((SwapChainWGPU&)swapChain).AcquireNextTexture(textureIndex);
}

static Result WaitForPresent(SwapChain& swapChain) {
    return ((SwapChainWGPU&)swapChain).WaitForPresent();
}

static Result QueuePresent(SwapChain& swapChain, Fence&) {
    return ((SwapChainWGPU&)swapChain).Present();
}

Result DeviceWGPU::FillFunctionTable(SwapChainInterface& table) const {
    if (!m_Desc.features.swapChain)
        return Result::UNSUPPORTED;

    table.CreateSwapChain = ::CreateSwapChain;
    table.DestroySwapChain = ::DestroySwapChain;
    table.GetSwapChainTextures = ::GetSwapChainTextures;
    table.GetDisplayDesc = ::GetDisplayDesc;
    table.AcquireNextTexture = ::AcquireNextTexture;
    table.WaitForPresent = ::WaitForPresent;
    table.QueuePresent = ::QueuePresent;

    return Result::Success;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  Upscaler  ]

Result DeviceWGPU::FillFunctionTable(UpscalerInterface&) const {
    // TODO: No WGPU mapping exists for the upscaler extension yet.
    return Result::UNSUPPORTED;
}

#pragma endregion
