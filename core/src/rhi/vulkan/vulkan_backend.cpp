// © 2021 NVIDIA Corporation

#include "memory_allocator_vulkan.hpp"

#include "shared_vulkan.hpp"

#include "acceleration_structure_vulkan.hpp"
#include "buffer_vulkan.hpp"
#include "command_allocator_vulkan.hpp"
#include "command_buffer_vulkan.hpp"
#include "conversion_vulkan.hpp"
#include "descriptor_pool_vulkan.hpp"
#include "descriptor_set_vulkan.hpp"
#include "descriptor_vulkan.hpp"
#include "fence_vulkan.hpp"
#include "memory_vulkan.hpp"
#include "micromap_vulkan.hpp"
#include "pipeline_cache_vulkan.hpp"
#include "pipeline_layout_vulkan.hpp"
#include "pipeline_vulkan.hpp"
#include "query_pool_vulkan.hpp"
#include "queue_vulkan.hpp"
#include "swap_chain_vulkan.hpp"
#include "texture_vulkan.hpp"

#include <core/rhi/extensions/helper.hpp>
#include <core/rhi/extensions/imgui.hpp>
#include <core/rhi/extensions/streamer.hpp>
#include <core/rhi/extensions/upscaler.hpp>

using namespace Core::RHI;

#include "acceleration_structure_vulkan.hpp"
#include "buffer_vulkan.hpp"
#include "command_allocator_vulkan.hpp"
#include "command_buffer_vulkan.hpp"
#include "conversion_vulkan.hpp"
#include "descriptor_pool_vulkan.hpp"
#include "descriptor_set_vulkan.hpp"
#include "descriptor_vulkan.hpp"
#include "device_vulkan.hpp"
#include "fence_vulkan.hpp"
#include "memory_vulkan.hpp"
#include "micromap_vulkan.hpp"
#include "pipeline_cache_vulkan.hpp"
#include "pipeline_layout_vulkan.hpp"
#include "pipeline_vulkan.hpp"
#include "query_pool_vulkan.hpp"
#include "queue_vulkan.hpp"
#include "swap_chain_vulkan.hpp"
#include "texture_vulkan.hpp"

namespace Core::RHI {
  Result createVulkanDevice(const DeviceCreationDesc &desc, const DeviceCreationVKDesc &descVK, DeviceBase *&device) {
    DeviceVK *impl = Allocate<DeviceVK>(desc.allocationCallbacks, desc.callbackInterface, desc.allocationCallbacks);
    Result result = impl->Create(desc, descVK);

    if (result != Result::Success) {
      Destroy(desc.allocationCallbacks, impl);
      device = nullptr;
    }
    else
      device = (DeviceBase*)impl;

    return result;
  }

  //============================================================================================================================================================================================
#pragma region[  Core  ]

  static const DeviceInfo &GetDeviceInfo(const Device &device) {
    return ((DeviceVK&)device).GetDesc();
  }

  static const BufferDesc &GetBufferDesc(const Buffer &buffer) {
    return ((BufferVK&)buffer).GetDesc();
  }

  static const TextureDesc &GetTextureDesc(const Texture &texture) {
    return ((TextureVK&)texture).GetDesc();
  }

  static FormatSupportBits GetFormatSupport(const Device &device, Format format) {
    return ((DeviceVK&)device).GetFormatSupport(format);
  }

  static Result GetQueue(Device &device, QueueType queueType, uint32_t queueIndex, Queue *&queue) {
    return ((DeviceVK&)device).GetQueue(queueType, queueIndex, queue);
  }

  static Result CreateCommandAllocator(Queue &queue, CommandAllocator *&commandAllocator) {
    DeviceVK &device = ((QueueVK&)queue).GetDevice();
    return device.CreateImplementation<CommandAllocatorVK>(commandAllocator, queue);
  }

  static Result CreateCommandBuffer(CommandAllocator &commandAllocator, CommandBuffer *&commandBuffer) {
    return ((CommandAllocatorVK&)commandAllocator).CreateCommandBuffer(commandBuffer);
  }

  static Result CreateFence(Device &device, uint64_t initialValue, Fence *&fence) {
    return ((DeviceVK&)device).CreateImplementation<FenceVK>(fence, initialValue);
  }

  static Result CreateDescriptorPool(Device &device, const DescriptorPoolDesc &descriptorPoolDesc,
                                     DescriptorPool *&descriptorPool) {
    return ((DeviceVK&)device).CreateImplementation<DescriptorPoolVK>(descriptorPool, descriptorPoolDesc);
  }

  static Result CreatePipelineLayout(Device &device, const PipelineLayoutDesc &pipelineLayoutDesc,
                                     PipelineLayout *&pipelineLayout) {
    return ((DeviceVK&)device).CreateImplementation<PipelineLayoutVK>(pipelineLayout, pipelineLayoutDesc);
  }

  static Result CreateGraphicsPipeline(Device &device, const GraphicsPipelineDesc &graphicsPipelineDesc,
                                       Pipeline *&pipeline) {
    return ((DeviceVK&)device).CreateImplementation<PipelineVK>(pipeline, graphicsPipelineDesc);
  }

  static Result CreateComputePipeline(Device &device, const ComputePipelineDesc &computePipelineDesc,
                                      Pipeline *&pipeline) {
    return ((DeviceVK&)device).CreateImplementation<PipelineVK>(pipeline, computePipelineDesc);
  }

  static Result CreatePipelineCache(Device &device, const PipelineCacheDesc &pipelineCacheDesc,
                                    PipelineCache *&pipelineCache) {
    return ((DeviceVK&)device).CreateImplementation<PipelineCacheVK>(pipelineCache, pipelineCacheDesc);
  }

  static Result CreateQueryPool(Device &device, const QueryPoolDesc &queryPoolDesc, QueryPool *&queryPool) {
    return ((DeviceVK&)device).CreateImplementation<QueryPoolVK>(queryPool, queryPoolDesc);
  }

  static Result CreateSampler(Device &device, const SamplerDesc &samplerDesc, Descriptor *&sampler) {
    return ((DeviceVK&)device).CreateImplementation<DescriptorVK>(sampler, samplerDesc);
  }

  static Result CreateBufferView(const BufferViewDesc &bufferViewDesc, Descriptor *&bufferView) {
    DeviceVK &device = ((BufferVK*)bufferViewDesc.buffer)->GetDevice();
    return device.CreateImplementation<DescriptorVK>(bufferView, bufferViewDesc);
  }

  static Result CreateTextureView(const TextureViewDesc &textureViewDesc, Descriptor *&textureView) {
    DeviceVK &device = ((TextureVK*)textureViewDesc.texture)->GetDevice();
    return device.CreateImplementation<DescriptorVK>(textureView, textureViewDesc);
  }

  static void DestroyCommandAllocator(CommandAllocator *commandAllocator) {
    Destroy((CommandAllocatorVK*)commandAllocator);
  }

  static void DestroyCommandBuffer(CommandBuffer *commandBuffer) {
    Destroy((CommandBufferVK*)commandBuffer);
  }

  static void DestroyDescriptorPool(DescriptorPool *descriptorPool) {
    Destroy((DescriptorPoolVK*)descriptorPool);
  }

  static void DestroyBuffer(Buffer *buffer) {
    Destroy((BufferVK*)buffer);
  }

  static void DestroyTexture(Texture *texture) {
    Destroy((TextureVK*)texture);
  }

  static void DestroyDescriptor(Descriptor *descriptor) {
    Destroy((DescriptorVK*)descriptor);
  }

  static void DestroyPipelineLayout(PipelineLayout *pipelineLayout) {
    Destroy((PipelineLayoutVK*)pipelineLayout);
  }

  static void DestroyPipeline(Pipeline *pipeline) {
    Destroy((PipelineVK*)pipeline);
  }

  static void DestroyPipelineCache(PipelineCache *pipelineCache) {
    Destroy((PipelineCacheVK*)pipelineCache);
  }

  static Result GetPipelineCacheData(PipelineCache &pipelineCache, void *dst, uint64_t &size) {
    return ((PipelineCacheVK&)pipelineCache).GetData(dst, size);
  }

  static void DestroyQueryPool(QueryPool *queryPool) {
    Destroy((QueryPoolVK*)queryPool);
  }

  static void DestroyFence(Fence *fence) {
    Destroy((FenceVK*)fence);
  }

  static Result AllocateMemory(Device &device, const AllocateMemoryDesc &allocateMemoryDesc, Memory *&memory) {
    return ((DeviceVK&)device).CreateImplementation<MemoryVK>(memory, allocateMemoryDesc);
  }

  static void FreeMemory(Memory *memory) {
    Destroy((MemoryVK*)memory);
  }

  static Result CreateBuffer(Device &device, const BufferDesc &bufferDesc, Buffer *&buffer) {
    return ((DeviceVK&)device).CreateImplementation<BufferVK>(buffer, bufferDesc);
  }

  static Result CreateTexture(Device &device, const TextureDesc &textureDesc, Texture *&texture) {
    return ((DeviceVK&)device).CreateImplementation<TextureVK>(texture, textureDesc);
  }

  static void GetBufferMemoryDesc(const Buffer &buffer, MemoryLocation memoryLocation, MemoryDesc &memoryDesc) {
    ((BufferVK&)buffer).GetMemoryDesc(memoryLocation, memoryDesc);
  }

  static void GetTextureMemoryDesc(const Texture &texture, MemoryLocation memoryLocation, MemoryDesc &memoryDesc) {
    ((TextureVK&)texture).GetMemoryDesc(memoryLocation, memoryDesc);
  }

  static Result BindBufferMemory(const BindBufferMemoryDesc *bindBufferMemoryDescs, uint32_t bindBufferMemoryDescNum) {
    if (!bindBufferMemoryDescNum)
      return Result::Success;

    DeviceVK &deviceVK = ((BufferVK*)bindBufferMemoryDescs->buffer)->GetDevice();
    return deviceVK.BindBufferMemory(bindBufferMemoryDescs, bindBufferMemoryDescNum);
  }

  static Result BindTextureMemory(const BindTextureMemoryDesc *bindTextureMemoryDescs,
                                  uint32_t bindTextureMemoryDescNum) {
    if (!bindTextureMemoryDescNum)
      return Result::Success;

    DeviceVK &deviceVK = ((TextureVK*)bindTextureMemoryDescs->texture)->GetDevice();
    return deviceVK.BindTextureMemory(bindTextureMemoryDescs, bindTextureMemoryDescNum);
  }

  static void GetBufferMemoryDesc2(const Device &device, const BufferDesc &bufferDesc, MemoryLocation memoryLocation,
                                   MemoryDesc &memoryDesc) {
    ((DeviceVK&)device).GetMemoryDesc2(bufferDesc, memoryLocation, memoryDesc);
  }

  static void GetTextureMemoryDesc2(const Device &device, const TextureDesc &textureDesc, MemoryLocation memoryLocation,
                                    MemoryDesc &memoryDesc) {
    ((DeviceVK&)device).GetMemoryDesc2(textureDesc, memoryLocation, memoryDesc);
  }

  static Result CreateCommittedBuffer(Device &device, MemoryLocation memoryLocation, float priority,
                                      const BufferDesc &bufferDesc, Buffer *&buffer) {
    DeviceVK &deviceVK = (DeviceVK&)device;

    Result result = deviceVK.CreateImplementation<BufferVK>(buffer, bufferDesc);
    if (result != Result::Success)
      return result;

    return ((BufferVK*)buffer)->AllocateAndBindMemory(memoryLocation, priority, true);
  }

  static Result CreateCommittedTexture(Device &device, MemoryLocation memoryLocation, float priority,
                                       const TextureDesc &textureDesc, Texture *&texture) {
    DeviceVK &deviceVK = (DeviceVK&)device;

    Result result = deviceVK.CreateImplementation<TextureVK>(texture, textureDesc);
    if (result != Result::Success)
      return result;

    return ((TextureVK*)texture)->AllocateAndBindMemory(memoryLocation, priority, true);
  }

  static Result CreatePlacedBuffer(Device &device, Memory *memory, uint64_t offset, const BufferDesc &bufferDesc,
                                   Buffer *&buffer) {
    DeviceVK &deviceVK = (DeviceVK&)device;

    Result result = deviceVK.CreateImplementation<BufferVK>(buffer, bufferDesc);
    if (result != Result::Success)
      return result;

    if (memory)
      result = ((BufferVK*)buffer)->BindMemory(*(MemoryVK*)memory, offset, true);
    else
      result = ((BufferVK*)buffer)->AllocateAndBindMemory((MemoryLocation)offset, 0.0f, false);

    return result;
  }

  static Result CreatePlacedTexture(Device &device, Memory *memory, uint64_t offset, const TextureDesc &textureDesc,
                                    Texture *&texture) {
    DeviceVK &deviceVK = (DeviceVK&)device;

    Result result = deviceVK.CreateImplementation<TextureVK>(texture, textureDesc);
    if (result != Result::Success)
      return result;

    if (memory)
      result = ((TextureVK*)texture)->BindMemory(*(MemoryVK*)memory, offset);
    else
      result = ((TextureVK*)texture)->AllocateAndBindMemory((MemoryLocation)offset, 0.0f, false);

    return result;
  }

  static Result AllocateDescriptorSets(DescriptorPool &descriptorPool, const PipelineLayout &pipelineLayout,
                                       uint32_t setIndex, DescriptorSet **descriptorSets, uint32_t instanceNum,
                                       uint32_t variableDescriptorNum) {
    return ((DescriptorPoolVK&)descriptorPool).AllocateDescriptorSets(pipelineLayout, setIndex, descriptorSets,
                                                                      instanceNum, variableDescriptorNum);
  }

  static void UpdateDescriptorRanges(const UpdateDescriptorRangeDesc *updateDescriptorRangeDescs,
                                     uint32_t updateDescriptorRangeDescNum) {
    if (!updateDescriptorRangeDescNum)
      return;

    DeviceVK &deviceVK = ((DescriptorSetVK*)updateDescriptorRangeDescs->descriptorSet)->GetDevice();
    deviceVK.UpdateDescriptorRanges(updateDescriptorRangeDescs, updateDescriptorRangeDescNum);
  }

  static void CopyDescriptorRanges(const CopyDescriptorRangeDesc *copyDescriptorRangeDescs,
                                   uint32_t copyDescriptorRangeDescNum) {
    if (!copyDescriptorRangeDescNum)
      return;

    DeviceVK &deviceVK = ((DescriptorSetVK*)copyDescriptorRangeDescs->dstDescriptorSet)->GetDevice();
    deviceVK.CopyDescriptorRanges(copyDescriptorRangeDescs, copyDescriptorRangeDescNum);
  }

  static void GetDescriptorSetOffsets(const DescriptorSet &, uint32_t &resourceHeapOffset,
                                      uint32_t &samplerHeapOffset) {
    resourceHeapOffset = 0;
    samplerHeapOffset = 0;
  }

  static void ResetDescriptorPool(DescriptorPool &descriptorPool) {
    ((DescriptorPoolVK&)descriptorPool).Reset();
  }

  static Result BeginCommandBuffer(CommandBuffer &commandBuffer, const DescriptorPool *descriptorPool) {
    return ((CommandBufferVK&)commandBuffer).Begin(descriptorPool);
  }

  static void CmdSetDescriptorPool(CommandBuffer &, const DescriptorPool &) {
  }

  static void CmdSetPipelineLayout(CommandBuffer &commandBuffer, BindPoint bindPoint,
                                   const PipelineLayout &pipelineLayout) {
    ((CommandBufferVK&)commandBuffer).SetPipelineLayout(bindPoint, pipelineLayout);
  }

  static void CmdSetDescriptorSet(CommandBuffer &commandBuffer, const SetDescriptorSetDesc &setDescriptorSetDesc) {
    ((CommandBufferVK&)commandBuffer).SetDescriptorSet(setDescriptorSetDesc);
  }

  static void CmdSetRootConstants(CommandBuffer &commandBuffer, const SetRootConstantsDesc &setRootConstantsDesc) {
    ((CommandBufferVK&)commandBuffer).SetRootConstants(setRootConstantsDesc);
  }

  static void CmdSetRootDescriptor(CommandBuffer &commandBuffer, const SetRootDescriptorDesc &setRootDescriptorDesc) {
    ((CommandBufferVK&)commandBuffer).SetRootDescriptor(setRootDescriptorDesc);
  }

  static void CmdSetPipeline(CommandBuffer &commandBuffer, const Pipeline &pipeline) {
    ((CommandBufferVK&)commandBuffer).SetPipeline(pipeline);
  }

  static void CmdBarrier(CommandBuffer &commandBuffer, const BarrierDesc &barrierDesc) {
    ((CommandBufferVK&)commandBuffer).Barrier(barrierDesc);
  }

  static void CmdSetIndexBuffer(CommandBuffer &commandBuffer, const Buffer &buffer, uint64_t offset,
                                IndexType indexType) {
    ((CommandBufferVK&)commandBuffer).SetIndexBuffer(buffer, offset, indexType);
  }

  static void CmdSetVertexBuffers(CommandBuffer &commandBuffer, uint32_t baseSlot,
                                  const VertexBufferDesc *vertexBufferDescs, uint32_t vertexBufferNum) {
    ((CommandBufferVK&)commandBuffer).SetVertexBuffers(baseSlot, vertexBufferDescs, vertexBufferNum);
  }

  static void CmdSetViewports(CommandBuffer &commandBuffer, const Viewport *viewports, uint32_t viewportNum) {
    ((CommandBufferVK&)commandBuffer).SetViewports(viewports, viewportNum);
  }

  static void CmdSetScissors(CommandBuffer &commandBuffer, const Rect *rects, uint32_t rectNum) {
    ((CommandBufferVK&)commandBuffer).SetScissors(rects, rectNum);
  }

  static void CmdSetStencilReference(CommandBuffer &commandBuffer, uint8_t frontRef, uint8_t backRef) {
    ((CommandBufferVK&)commandBuffer).SetStencilReference(frontRef, backRef);
  }

  static void CmdSetDepthBounds(CommandBuffer &commandBuffer, float boundsMin, float boundsMax) {
    ((CommandBufferVK&)commandBuffer).SetDepthBounds(boundsMin, boundsMax);
  }

  static void CmdSetBlendConstants(CommandBuffer &commandBuffer, const Color32f &color) {
    ((CommandBufferVK&)commandBuffer).SetBlendConstants(color);
  }

  static void CmdSetSampleLocations(CommandBuffer &commandBuffer, const SampleLocation *locations, Sample_t locationNum,
                                    Sample_t sampleNum) {
    ((CommandBufferVK&)commandBuffer).SetSampleLocations(locations, locationNum, sampleNum);
  }

  static void CmdSetShadingRate(CommandBuffer &commandBuffer, const ShadingRateDesc &shadingRateDesc) {
    ((CommandBufferVK&)commandBuffer).SetShadingRate(shadingRateDesc);
  }

  static void CmdSetDepthBias(CommandBuffer &commandBuffer, const DepthBiasDesc &depthBiasDesc) {
    ((CommandBufferVK&)commandBuffer).SetDepthBias(depthBiasDesc);
  }

  static void CmdBeginRendering(CommandBuffer &commandBuffer, const RenderingDesc &renderingDesc) {
    ((CommandBufferVK&)commandBuffer).BeginRendering(renderingDesc);
  }

  static void CmdClearAttachments(CommandBuffer &commandBuffer, const ClearAttachmentDesc *clearAttachmentDescs,
                                  uint32_t clearAttachmentDescNum, const Rect *rects, uint32_t rectNum) {
    ((CommandBufferVK&)commandBuffer).ClearAttachments(clearAttachmentDescs, clearAttachmentDescNum, rects, rectNum);
  }

  static void CmdDraw(CommandBuffer &commandBuffer, const DrawDesc &drawDesc) {
    ((CommandBufferVK&)commandBuffer).Draw(drawDesc);
  }

  static void CmdDrawIndexed(CommandBuffer &commandBuffer, const DrawIndexedDesc &drawIndexedDesc) {
    ((CommandBufferVK&)commandBuffer).DrawIndexed(drawIndexedDesc);
  }

  static void CmdDrawIndirect(CommandBuffer &commandBuffer, const Buffer &buffer, uint64_t offset, uint32_t drawNum,
                              uint32_t stride, const Buffer *countBuffer, uint64_t countBufferOffset) {
    ((CommandBufferVK&)commandBuffer).DrawIndirect(buffer, offset, drawNum, stride, countBuffer, countBufferOffset);
  }

  static void CmdDrawIndexedIndirect(CommandBuffer &commandBuffer, const Buffer &buffer, uint64_t offset,
                                     uint32_t drawNum, uint32_t stride, const Buffer *countBuffer,
                                     uint64_t countBufferOffset) {
    ((CommandBufferVK&)commandBuffer).DrawIndexedIndirect(buffer, offset, drawNum, stride, countBuffer,
                                                          countBufferOffset);
  }

  static void CmdEndRendering(CommandBuffer &commandBuffer) {
    ((CommandBufferVK&)commandBuffer).EndRendering();
  }

  static void CmdDispatch(CommandBuffer &commandBuffer, const DispatchDesc &dispatchDesc) {
    ((CommandBufferVK&)commandBuffer).Dispatch(dispatchDesc);
  }

  static void CmdDispatchIndirect(CommandBuffer &commandBuffer, const Buffer &buffer, uint64_t offset) {
    ((CommandBufferVK&)commandBuffer).DispatchIndirect(buffer, offset);
  }

  static void CmdCopyBuffer(CommandBuffer &commandBuffer, Buffer &dstBuffer, uint64_t dstOffset,
                            const Buffer &srcBuffer, uint64_t srcOffset, uint64_t size) {
    ((CommandBufferVK&)commandBuffer).CopyBuffer(dstBuffer, dstOffset, srcBuffer, srcOffset, size);
  }

  static void CmdCopyTexture(CommandBuffer &commandBuffer, Texture &dstTexture, const TextureRegionDesc *dstRegion,
                             const Texture &srcTexture, const TextureRegionDesc *srcRegion) {
    ((CommandBufferVK&)commandBuffer).CopyTexture(dstTexture, dstRegion, srcTexture, srcRegion);
  }

  static void CmdUploadBufferToTexture(CommandBuffer &commandBuffer, Texture &dstTexture,
                                       const TextureRegionDesc &dstRegion, const Buffer &srcBuffer,
                                       const TextureDataLayoutDesc &srcDataLayout) {
    ((CommandBufferVK&)commandBuffer).UploadBufferToTexture(dstTexture, dstRegion, srcBuffer, srcDataLayout);
  }

  static void CmdReadbackTextureToBuffer(CommandBuffer &commandBuffer, Buffer &dstBuffer,
                                         const TextureDataLayoutDesc &dstDataLayout, const Texture &srcTexture,
                                         const TextureRegionDesc &srcRegion) {
    ((CommandBufferVK&)commandBuffer).ReadbackTextureToBuffer(dstBuffer, dstDataLayout, srcTexture, srcRegion);
  }

  static void CmdZeroBuffer(CommandBuffer &commandBuffer, Buffer &buffer, uint64_t offset, uint64_t size) {
    ((CommandBufferVK&)commandBuffer).ZeroBuffer(buffer, offset, size);
  }

  static void CmdResolveTexture(CommandBuffer &commandBuffer, Texture &dstTexture, const TextureRegionDesc *dstRegion,
                                const Texture &srcTexture, const TextureRegionDesc *srcRegion, ResolveOp resolveOp) {
    ((CommandBufferVK&)commandBuffer).ResolveTexture(dstTexture, dstRegion, srcTexture, srcRegion, resolveOp);
  }

  static void CmdClearStorage(CommandBuffer &commandBuffer, const ClearStorageDesc &clearStorageDesc) {
    ((CommandBufferVK&)commandBuffer).ClearStorage(clearStorageDesc);
  }

  static void CmdResetQueries(CommandBuffer &commandBuffer, QueryPool &queryPool, uint32_t offset, uint32_t num) {
    ((CommandBufferVK&)commandBuffer).ResetQueries(queryPool, offset, num);
  }

  static void CmdBeginQuery(CommandBuffer &commandBuffer, QueryPool &queryPool, uint32_t offset) {
    ((CommandBufferVK&)commandBuffer).BeginQuery(queryPool, offset);
  }

  static void CmdEndQuery(CommandBuffer &commandBuffer, QueryPool &queryPool, uint32_t offset) {
    ((CommandBufferVK&)commandBuffer).EndQuery(queryPool, offset);
  }

  static void CmdCopyQueries(CommandBuffer &commandBuffer, const QueryPool &queryPool, uint32_t offset, uint32_t num,
                             Buffer &dstBuffer, uint64_t dstOffset) {
    ((CommandBufferVK&)commandBuffer).CopyQueries(queryPool, offset, num, dstBuffer, dstOffset);
  }

  static void CmdBeginAnnotation(CommandBuffer &commandBuffer, const char *name, uint32_t bgra) {
    MaybeUnused(commandBuffer, name, bgra);
#if NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS
    ((CommandBufferVK&)commandBuffer).BeginAnnotation(name, bgra);
#endif
  }

  static void CmdEndAnnotation(CommandBuffer &commandBuffer) {
    MaybeUnused(commandBuffer);
#if NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS
    ((CommandBufferVK&)commandBuffer).EndAnnotation();
#endif
  }

  static void CmdAnnotation(CommandBuffer &commandBuffer, const char *name, uint32_t bgra) {
    MaybeUnused(commandBuffer, name, bgra);
#if NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS
    ((CommandBufferVK&)commandBuffer).Annotation(name, bgra);
#endif
  }

  static Result EndCommandBuffer(CommandBuffer &commandBuffer) {
    return ((CommandBufferVK&)commandBuffer).End();
  }

  static void QueueBeginAnnotation(Queue &queue, const char *name, uint32_t bgra) {
    MaybeUnused(queue, name, bgra);
#if NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS
    ((QueueVK&)queue).BeginAnnotation(name, bgra);
#endif
  }

  static void QueueEndAnnotation(Queue &queue) {
    MaybeUnused(queue);
#if NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS
    ((QueueVK&)queue).EndAnnotation();
#endif
  }

  static void QueueAnnotation(Queue &queue, const char *name, uint32_t bgra) {
    MaybeUnused(queue, name, bgra);
#if NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS
    ((QueueVK&)queue).Annotation(name, bgra);
#endif
  }

  static void ResetQueries(QueryPool &queryPool, uint32_t offset, uint32_t num) {
    ((QueryPoolVK&)queryPool).Reset(offset, num);
  }

  static uint32_t GetQuerySize(const QueryPool &queryPool) {
    return ((QueryPoolVK&)queryPool).GetQuerySize();
  }

  static void GetCalibratedTimestamps(Queue &queue, uint64_t &timestampGPU, uint64_t &timestampCPU) {
    ((QueueVK&)queue).GetCalibratedTimestamps(timestampGPU, timestampCPU);
  }

  static Result QueueSubmit(Queue &queue, const QueueSubmitDesc &workSubmissionDesc) {
    return ((QueueVK&)queue).Submit(workSubmissionDesc);
  }

  static Result QueueWaitIdle(Queue *queue) {
    if (!queue)
      return Result::Success;

    return ((QueueVK*)queue)->WaitIdle();
  }

  static Result DeviceWaitIdle(Device *device) {
    if (!device)
      return Result::Success;

    return ((DeviceVK*)device)->WaitIdle();
  }

  static void Wait(Fence &fence, uint64_t value) {
    ((FenceVK&)fence).Wait(value);
  }

  static uint64_t GetFenceValue(Fence &fence) {
    return ((FenceVK&)fence).GetFenceValue();
  }

  static void ResetCommandAllocator(CommandAllocator &commandAllocator) {
    ((CommandAllocatorVK&)commandAllocator).Reset();
  }

  static void *MapBuffer(Buffer &buffer, uint64_t offset, uint64_t size) {
    return ((BufferVK&)buffer).Map(offset, size);
  }

  static void UnmapBuffer(Buffer &buffer) {
    ((BufferVK&)buffer).Unmap();
  }

  static uint64_t GetBufferDeviceAddress(const Buffer &buffer) {
    return ((BufferVK&)buffer).GetDeviceAddress();
  }

  static void SetDebugName(Object *object, const char *name) {
    MaybeUnused(object, name);
#if NRI_ENABLE_DEBUG_NAMES_AND_ANNOTATIONS
    if (object)
      ((DebugNameBase*)object)->SetDebugName(name);
#endif
  }

  static void *GetDeviceNativeObject(const Device *device) {
    if (!device)
      return nullptr;

    return (VkDevice)(*(DeviceVK*)device);
  }

  static void *GetQueueNativeObject(const Queue *queue) {
    if (!queue)
      return nullptr;

    return (VkQueue)(*(QueueVK*)queue);
  }

  static void *GetCommandBufferNativeObject(const CommandBuffer *commandBuffer) {
    if (!commandBuffer)
      return nullptr;

    return (VkCommandBuffer)(*(CommandBufferVK*)commandBuffer);
  }

  static uint64_t GetBufferNativeObject(const Buffer *buffer) {
    if (!buffer)
      return 0;

    return uint64_t(((BufferVK*)buffer)->GetHandle());
  }

  static uint64_t GetTextureNativeObject(const Texture *texture) {
    if (!texture)
      return 0;

    return uint64_t(((TextureVK*)texture)->GetHandle());
  }

  static uint64_t GetDescriptorNativeObject(const Descriptor *descriptor) {
    if (!descriptor)
      return 0;

    const DescriptorVK &d = *(DescriptorVK*)descriptor;
    DescriptorType descriptorType = d.GetType();

    uint64_t handle = 0;
    switch (descriptorType) {
      case DescriptorType::SAMPLER:
        handle = (uint64_t)d.GetSampler();
        break;
      case DescriptorType::BUFFER:
      case DescriptorType::STORAGE_BUFFER:
      case DescriptorType::CONSTANT_BUFFER:
      case DescriptorType::STRUCTURED_BUFFER:
      case DescriptorType::STORAGE_STRUCTURED_BUFFER:
        handle = (uint64_t)d.GetBufferView(); // 0 for non-typed views
        break;
      case DescriptorType::ACCELERATION_STRUCTURE:
        handle = (uint64_t)d.GetAccelerationStructure();
        break;
      default: // all textures (including HOST only)
        handle = (uint64_t)d.GetImageView();
        break;
    }

    return handle;
  }

  Result DeviceVK::FillFunctionTable(CoreInterface &table) const {
    table.GetDeviceInfo = ::GetDeviceInfo;
    table.GetBufferDesc = ::GetBufferDesc;
    table.GetTextureDesc = ::GetTextureDesc;
    table.GetFormatSupport = ::GetFormatSupport;
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
    table.ResetQueries = ::ResetQueries;
    table.GetQuerySize = ::GetQuerySize;
    table.GetCalibratedTimestamps = ::GetCalibratedTimestamps;
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

  static Result UploadData(Queue &queue, const TextureUploadDesc *textureUploadDescs, uint32_t textureUploadDescNum,
                           const BufferUploadDesc *bufferUploadDescs, uint32_t bufferUploadDescNum) {
    QueueVK &queueVK = (QueueVK&)queue;
    DeviceVK &deviceVK = queueVK.GetDevice();
    HelperDataUpload helperDataUpload(deviceVK.GetCoreInterface(), (Device&)deviceVK, queue);

    return helperDataUpload.UploadData(textureUploadDescs, textureUploadDescNum, bufferUploadDescs,
                                       bufferUploadDescNum);
  }

  static uint32_t CalculateAllocationNumber(const Device &device, const ResourceGroupDesc &resourceGroupDesc) {
    DeviceVK &deviceVK = (DeviceVK&)device;
    HelperDeviceMemoryAllocator allocator(deviceVK.GetCoreInterface(), (Device&)device);

    return allocator.CalculateAllocationNumber(resourceGroupDesc);
  }

  static Result AllocateAndBindMemory(Device &device, const ResourceGroupDesc &resourceGroupDesc,
                                      Memory **allocations) {
    DeviceVK &deviceVK = (DeviceVK&)device;
    HelperDeviceMemoryAllocator allocator(deviceVK.GetCoreInterface(), device);

    return allocator.AllocateAndBindMemory(resourceGroupDesc, allocations);
  }

  static Result QueryVideoMemoryInfo(const Device &device, MemoryLocation memoryLocation,
                                     VideoMemoryInfo &videoMemoryInfo) {
    return ((DeviceVK&)device).QueryVideoMemoryInfo(memoryLocation, videoMemoryInfo);
  }

  Result DeviceVK::FillFunctionTable(HelperInterface &table) const {
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

  static Result CreateImgui(Device &device, const ImguiDesc &imguiDesc, Imgui *&imgui) {
    DeviceVK &deviceVK = (DeviceVK&)device;
    ImguiImpl *impl = Allocate<ImguiImpl>(deviceVK.GetAllocationCallbacks(), device, deviceVK.GetCoreInterface());
    Result result = impl->Create(imguiDesc);

    if (result != Result::Success) {
      Destroy(impl);
      imgui = nullptr;
    }
    else
      imgui = (Imgui*)impl;

    return result;
  }

  static void DestroyImgui(Imgui *imgui) {
    Destroy((ImguiImpl*)imgui);
  }

  static void CmdCopyImguiData(CommandBuffer &commandBuffer, Streamer &streamer, Imgui &imgui,
                               const CopyImguiDataDesc &copyImguiDataDesc) {
    ImguiImpl &imguiImpl = (ImguiImpl&)imgui;

    return imguiImpl.CmdCopyData(commandBuffer, streamer, copyImguiDataDesc);
  }

  static void CmdDrawImgui(CommandBuffer &commandBuffer, Imgui &imgui, const DrawImguiDesc &drawImguiDesc) {
    ImguiImpl &imguiImpl = (ImguiImpl&)imgui;

    return imguiImpl.CmdDraw(commandBuffer, drawImguiDesc);
  }

  Result DeviceVK::FillFunctionTable(ImguiInterface &table) const {
    table.CreateImgui = ::CreateImgui;
    table.DestroyImgui = ::DestroyImgui;
    table.CmdCopyImguiData = ::CmdCopyImguiData;
    table.CmdDrawImgui = ::CmdDrawImgui;

    return Result::Success;
  }

#endif

#pragma endregion

  //============================================================================================================================================================================================
#pragma region[  Low latency  ]

  static Result SetLatencySleepMode(SwapChain &swapChain, const LatencySleepMode &latencySleepMode) {
    return ((SwapChainVK&)swapChain).SetLatencySleepMode(latencySleepMode);
  }

  static Result SetLatencyMarker(SwapChain &swapChain, LatencyMarker latencyMarker) {
    return ((SwapChainVK&)swapChain).SetLatencyMarker(latencyMarker);
  }

  static Result LatencySleep(SwapChain &swapChain) {
    return ((SwapChainVK&)swapChain).LatencySleep();
  }

  static Result GetLatencyReport(const SwapChain &swapChain, LatencyReport &latencyReport) {
    return ((SwapChainVK&)swapChain).GetLatencyReport(latencyReport);
  }

  Result DeviceVK::FillFunctionTable(LowLatencyInterface &table) const {
    if (!m_Desc.features.lowLatency)
      return Result::UNSUPPORTED;

    table.SetLatencySleepMode = ::SetLatencySleepMode;
    table.SetLatencyMarker = ::SetLatencyMarker;
    table.LatencySleep = ::LatencySleep;
    table.GetLatencyReport = ::GetLatencyReport;

    return Result::Success;
  }

#pragma endregion

  //============================================================================================================================================================================================
#pragma region[  MeshShader  ]

  static void CmdDrawMeshTasks(CommandBuffer &commandBuffer, const DrawMeshTasksDesc &drawMeshTasksDesc) {
    ((CommandBufferVK&)commandBuffer).DrawMeshTasks(drawMeshTasksDesc);
  }

  static void CmdDrawMeshTasksIndirect(CommandBuffer &commandBuffer, const Buffer &buffer, uint64_t offset,
                                       uint32_t drawNum, uint32_t stride, const Buffer *countBuffer,
                                       uint64_t countBufferOffset) {
    ((CommandBufferVK&)commandBuffer).DrawMeshTasksIndirect(buffer, offset, drawNum, stride, countBuffer,
                                                            countBufferOffset);
  }

  Result DeviceVK::FillFunctionTable(MeshShaderInterface &table) const {
    if (!m_Desc.features.meshShader)
      return Result::UNSUPPORTED;

    table.CmdDrawMeshTasks = ::CmdDrawMeshTasks;
    table.CmdDrawMeshTasksIndirect = ::CmdDrawMeshTasksIndirect;

    return Result::Success;
  }

#pragma endregion

  //============================================================================================================================================================================================
#pragma region[  RayTracing  ]

  static Result CreateRayTracingPipeline(Device &device, const RayTracingPipelineDesc &pipelineDesc,
                                         Pipeline *&pipeline) {
    return ((DeviceVK&)device).CreateImplementation<PipelineVK>(pipeline, pipelineDesc);
  }

  static Result CreateAccelerationStructureDescriptor(const AccelerationStructure &accelerationStructure,
                                                      Descriptor *&descriptor) {
    return ((AccelerationStructureVK&)accelerationStructure).CreateDescriptor(descriptor);
  }

  static uint64_t GetAccelerationStructureHandle(const AccelerationStructure &accelerationStructure) {
    static_assert(sizeof(uint64_t) == sizeof(VkDeviceAddress), "type mismatch");
    return (uint64_t)((AccelerationStructureVK&)accelerationStructure).GetDeviceAddress();
  }

  static uint64_t GetAccelerationStructureUpdateScratchBufferSize(const AccelerationStructure &accelerationStructure) {
    return ((AccelerationStructureVK&)accelerationStructure).GetUpdateScratchBufferSize();
  }

  static uint64_t GetAccelerationStructureBuildScratchBufferSize(const AccelerationStructure &accelerationStructure) {
    return ((AccelerationStructureVK&)accelerationStructure).GetBuildScratchBufferSize();
  }

  static uint64_t GetMicromapBuildScratchBufferSize(const Micromap &micromap) {
    return ((MicromapVK&)micromap).GetBuildScratchBufferSize();
  }

  static Buffer *GetAccelerationStructureBuffer(const AccelerationStructure &accelerationStructure) {
    return (Buffer*)((AccelerationStructureVK&)accelerationStructure).GetBuffer();
  }

  static Buffer *GetMicromapBuffer(const Micromap &micromap) {
    return (Buffer*)((MicromapVK&)micromap).GetBuffer();
  }

  static void DestroyAccelerationStructure(AccelerationStructure *accelerationStructure) {
    Destroy((AccelerationStructureVK*)accelerationStructure);
  }

  static void DestroyMicromap(Micromap *micromap) {
    Destroy((MicromapVK*)micromap);
  }

  static Result CreateAccelerationStructure(Device &device, const AccelerationStructureDesc &accelerationStructureDesc,
                                            AccelerationStructure *&accelerationStructure) {
    return ((DeviceVK&)device).CreateImplementation<AccelerationStructureVK>(
      accelerationStructure, accelerationStructureDesc);
  }

  static Result CreateMicromap(Device &device, const MicromapDesc &micromapDesc, Micromap *&micromap) {
    return ((DeviceVK&)device).CreateImplementation<MicromapVK>(micromap, micromapDesc);
  }

  static void GetAccelerationStructureMemoryDesc(const AccelerationStructure &accelerationStructure,
                                                 MemoryLocation memoryLocation, MemoryDesc &memoryDesc) {
    ((AccelerationStructureVK&)accelerationStructure).GetBuffer()->GetMemoryDesc(memoryLocation, memoryDesc);
  }

  static void GetMicromapMemoryDesc(const Micromap &micromap, MemoryLocation memoryLocation, MemoryDesc &memoryDesc) {
    ((MicromapVK&)micromap).GetBuffer()->GetMemoryDesc(memoryLocation, memoryDesc);
  }

  static Result BindAccelerationStructureMemory(
    const BindAccelerationStructureMemoryDesc *bindAccelerationStructureMemoryDescs,
    uint32_t bindAccelerationStructureMemoryDescNum) {
    if (!bindAccelerationStructureMemoryDescNum)
      return Result::Success;

    DeviceVK &deviceVK = ((AccelerationStructureVK*)bindAccelerationStructureMemoryDescs->accelerationStructure)->
      GetDevice();
    return deviceVK.BindAccelerationStructureMemory(bindAccelerationStructureMemoryDescs,
                                                    bindAccelerationStructureMemoryDescNum);
  }

  static Result BindMicromapMemory(const BindMicromapMemoryDesc *bindMicromapMemoryDescs,
                                   uint32_t bindMicromapMemoryDescNum) {
    if (!bindMicromapMemoryDescNum)
      return Result::Success;

    DeviceVK &deviceVK = ((MicromapVK*)bindMicromapMemoryDescs->micromap)->GetDevice();
    return deviceVK.BindMicromapMemory(bindMicromapMemoryDescs, bindMicromapMemoryDescNum);
  }

  static void GetAccelerationStructureMemoryDesc2(const Device &device,
                                                  const AccelerationStructureDesc &accelerationStructureDesc,
                                                  MemoryLocation memoryLocation, MemoryDesc &memoryDesc) {
    ((DeviceVK&)device).GetMemoryDesc2(accelerationStructureDesc, memoryLocation, memoryDesc);
  }

  static void GetMicromapMemoryDesc2(const Device &device, const MicromapDesc &micromapDesc,
                                     MemoryLocation memoryLocation, MemoryDesc &memoryDesc) {
    ((DeviceVK&)device).GetMemoryDesc2(micromapDesc, memoryLocation, memoryDesc);
  }

  static Result CreateCommittedAccelerationStructure(Device &device, MemoryLocation memoryLocation, float priority,
                                                     const AccelerationStructureDesc &accelerationStructureDesc,
                                                     AccelerationStructure *&accelerationStructure) {
    DeviceVK &deviceVK = (DeviceVK&)device;

    Result result = deviceVK.CreateImplementation<AccelerationStructureVK>(
      accelerationStructure, accelerationStructureDesc);
    if (result != Result::Success)
      return result;

    return ((AccelerationStructureVK*)accelerationStructure)->AllocateAndBindMemory(memoryLocation, priority, true);
  }

  static Result CreateCommittedMicromap(Device &device, MemoryLocation memoryLocation, float priority,
                                        const MicromapDesc &micromapDesc, Micromap *&micromap) {
    DeviceVK &deviceVK = (DeviceVK&)device;

    Result result = deviceVK.CreateImplementation<MicromapVK>(micromap, micromapDesc);
    if (result != Result::Success)
      return result;

    return ((MicromapVK*)micromap)->AllocateAndBindMemory(memoryLocation, priority, true);
  }

  static Result CreatePlacedAccelerationStructure(Device &device, Memory *memory, uint64_t offset,
                                                  const AccelerationStructureDesc &accelerationStructureDesc,
                                                  AccelerationStructure *&accelerationStructure) {
    DeviceVK &deviceVK = (DeviceVK&)device;

    Result result = deviceVK.CreateImplementation<AccelerationStructureVK>(
      accelerationStructure, accelerationStructureDesc);
    if (result != Result::Success)
      return result;

    if (memory)
      result = ((AccelerationStructureVK*)accelerationStructure)->BindMemory((MemoryVK*)memory, offset);
    else
      result = ((AccelerationStructureVK*)accelerationStructure)->AllocateAndBindMemory(
        (MemoryLocation)offset, 0.0f, false);

    return result;
  }

  static Result CreatePlacedMicromap(Device &device, Memory *memory, uint64_t offset, const MicromapDesc &micromapDesc,
                                     Micromap *&micromap) {
    DeviceVK &deviceVK = (DeviceVK&)device;

    Result result = deviceVK.CreateImplementation<MicromapVK>(micromap, micromapDesc);
    if (result != Result::Success)
      return result;

    if (memory)
      result = ((MicromapVK*)micromap)->BindMemory((MemoryVK*)memory, offset);
    else
      result = ((MicromapVK*)micromap)->AllocateAndBindMemory((MemoryLocation)offset, 0.0f, false);

    return result;
  }

  static Result WriteShaderGroupIdentifiers(const Pipeline &pipeline, uint32_t baseShaderGroupIndex,
                                            uint32_t shaderGroupNum, void *dst) {
    return ((PipelineVK&)pipeline).WriteShaderGroupIdentifiers(baseShaderGroupIndex, shaderGroupNum, dst);
  }

  static void CmdBuildTopLevelAccelerationStructures(CommandBuffer &commandBuffer,
                                                     const BuildTopLevelAccelerationStructureDesc *
                                                     buildTopLevelAccelerationStructureDescs,
                                                     uint32_t buildTopLevelAccelerationStructureDescNum) {
    ((CommandBufferVK&)commandBuffer).BuildTopLevelAccelerationStructures(
      buildTopLevelAccelerationStructureDescs, buildTopLevelAccelerationStructureDescNum);
  }

  static void CmdBuildBottomLevelAccelerationStructures(CommandBuffer &commandBuffer,
                                                        const BuildBottomLevelAccelerationStructureDesc *
                                                        buildBottomLevelAccelerationStructureDescs,
                                                        uint32_t buildBottomLevelAccelerationStructureDescNum) {
    ((CommandBufferVK&)commandBuffer).BuildBottomLevelAccelerationStructures(
      buildBottomLevelAccelerationStructureDescs, buildBottomLevelAccelerationStructureDescNum);
  }

  static void CmdBuildMicromaps(CommandBuffer &commandBuffer, const BuildMicromapDesc *buildMicromapDescs,
                                uint32_t buildMicromapDescNum) {
    ((CommandBufferVK&)commandBuffer).BuildMicromaps(buildMicromapDescs, buildMicromapDescNum);
  }

  static void CmdDispatchRays(CommandBuffer &commandBuffer, const DispatchRaysDesc &dispatchRaysDesc) {
    ((CommandBufferVK&)commandBuffer).DispatchRays(dispatchRaysDesc);
  }

  static void CmdDispatchRaysIndirect(CommandBuffer &commandBuffer, const Buffer &buffer, uint64_t offset) {
    ((CommandBufferVK&)commandBuffer).DispatchRaysIndirect(buffer, offset);
  }

  static void CmdWriteAccelerationStructuresSizes(CommandBuffer &commandBuffer,
                                                  const AccelerationStructure *const*accelerationStructures,
                                                  uint32_t accelerationStructureNum, QueryPool &queryPool,
                                                  uint32_t queryPoolOffset) {
    ((CommandBufferVK&)commandBuffer).WriteAccelerationStructuresSizes(accelerationStructures, accelerationStructureNum,
                                                                       queryPool, queryPoolOffset);
  }

  static void CmdWriteMicromapsSizes(CommandBuffer &commandBuffer, const Micromap *const*micromaps,
                                     uint32_t micromapNum, QueryPool &queryPool, uint32_t queryPoolOffset) {
    ((CommandBufferVK&)commandBuffer).WriteMicromapsSizes(micromaps, micromapNum, queryPool, queryPoolOffset);
  }

  static void CmdCopyAccelerationStructure(CommandBuffer &commandBuffer, AccelerationStructure &dst,
                                           const AccelerationStructure &src, CopyMode mode) {
    ((CommandBufferVK&)commandBuffer).CopyAccelerationStructure(dst, src, mode);
  }

  static void CmdCopyMicromap(CommandBuffer &commandBuffer, Micromap &dst, const Micromap &src, CopyMode copyMode) {
    ((CommandBufferVK&)commandBuffer).CopyMicromap(dst, src, copyMode);
  }

  static uint64_t GetAccelerationStructureNativeObject(const AccelerationStructure *accelerationStructure) {
    if (!accelerationStructure)
      return 0;

    return uint64_t(((AccelerationStructureVK*)accelerationStructure)->GetHandle());
  }

  static uint64_t GetMicromapNativeObject(const Micromap *micromap) {
    if (!micromap)
      return 0;

    return uint64_t(((MicromapVK*)micromap)->GetHandle());
  }

  Result DeviceVK::FillFunctionTable(RayTracingInterface &table) const {
    if (m_Desc.tiers.rayTracing == 0)
      return Result::UNSUPPORTED;

    table.CreateRayTracingPipeline = ::CreateRayTracingPipeline;
    table.CreateAccelerationStructureDescriptor = ::CreateAccelerationStructureDescriptor;
    table.GetAccelerationStructureHandle = ::GetAccelerationStructureHandle;
    table.GetAccelerationStructureUpdateScratchBufferSize = ::GetAccelerationStructureUpdateScratchBufferSize;
    table.GetAccelerationStructureBuildScratchBufferSize = ::GetAccelerationStructureBuildScratchBufferSize;
    table.GetMicromapBuildScratchBufferSize = ::GetMicromapBuildScratchBufferSize;
    table.GetAccelerationStructureBuffer = ::GetAccelerationStructureBuffer;
    table.GetMicromapBuffer = ::GetMicromapBuffer;
    table.DestroyAccelerationStructure = ::DestroyAccelerationStructure;
    table.DestroyMicromap = ::DestroyMicromap;
    table.CreateAccelerationStructure = ::CreateAccelerationStructure;
    table.CreateMicromap = ::CreateMicromap;
    table.GetAccelerationStructureMemoryDesc = ::GetAccelerationStructureMemoryDesc;
    table.GetMicromapMemoryDesc = ::GetMicromapMemoryDesc;
    table.BindAccelerationStructureMemory = ::BindAccelerationStructureMemory;
    table.BindMicromapMemory = ::BindMicromapMemory;
    table.GetAccelerationStructureMemoryDesc2 = ::GetAccelerationStructureMemoryDesc2;
    table.GetMicromapMemoryDesc2 = ::GetMicromapMemoryDesc2;
    table.CreateCommittedAccelerationStructure = ::CreateCommittedAccelerationStructure;
    table.CreateCommittedMicromap = ::CreateCommittedMicromap;
    table.CreatePlacedAccelerationStructure = ::CreatePlacedAccelerationStructure;
    table.CreatePlacedMicromap = ::CreatePlacedMicromap;
    table.WriteShaderGroupIdentifiers = ::WriteShaderGroupIdentifiers;
    table.CmdBuildTopLevelAccelerationStructures = ::CmdBuildTopLevelAccelerationStructures;
    table.CmdBuildBottomLevelAccelerationStructures = ::CmdBuildBottomLevelAccelerationStructures;
    table.CmdBuildMicromaps = ::CmdBuildMicromaps;
    table.CmdDispatchRays = ::CmdDispatchRays;
    table.CmdDispatchRaysIndirect = ::CmdDispatchRaysIndirect;
    table.CmdWriteAccelerationStructuresSizes = ::CmdWriteAccelerationStructuresSizes;
    table.CmdWriteMicromapsSizes = ::CmdWriteMicromapsSizes;
    table.CmdCopyAccelerationStructure = ::CmdCopyAccelerationStructure;
    table.CmdCopyMicromap = ::CmdCopyMicromap;
    table.GetAccelerationStructureNativeObject = ::GetAccelerationStructureNativeObject;
    table.GetMicromapNativeObject = ::GetMicromapNativeObject;

    return Result::Success;
  }

#pragma endregion

  //============================================================================================================================================================================================
#pragma region[  Streamer  ]

  static Result CreateStreamer(Device &device, const StreamerDesc &streamerDesc, Streamer *&streamer) {
    DeviceVK &deviceVK = (DeviceVK&)device;
    StreamerImpl *impl = Allocate<StreamerImpl>(deviceVK.GetAllocationCallbacks(), device, deviceVK.GetCoreInterface());
    Result result = impl->Create(streamerDesc);

    if (result != Result::Success) {
      Destroy(impl);
      streamer = nullptr;
    }
    else
      streamer = (Streamer*)impl;

    return result;
  }

  static void DestroyStreamer(Streamer *streamer) {
    Destroy((StreamerImpl*)streamer);
  }

  static Buffer *GetStreamerConstantBuffer(Streamer &streamer) {
    return ((StreamerImpl&)streamer).GetConstantBuffer();
  }

  static uint32_t StreamConstantData(Streamer &streamer, const void *data, uint32_t dataSize) {
    return ((StreamerImpl&)streamer).StreamConstantData(data, dataSize);
  }

  static BufferOffset StreamBufferData(Streamer &streamer, const StreamBufferDataDesc &streamBufferDataDesc) {
    return ((StreamerImpl&)streamer).StreamBufferData(streamBufferDataDesc);
  }

  static BufferOffset StreamTextureData(Streamer &streamer, const StreamTextureDataDesc &streamTextureDataDesc) {
    return ((StreamerImpl&)streamer).StreamTextureData(streamTextureDataDesc);
  }

  static void EndStreamerFrame(Streamer &streamer) {
    return ((StreamerImpl&)streamer).EndFrame();
  }

  static void CmdCopyStreamedData(CommandBuffer &commandBuffer, Streamer &streamer) {
    ((StreamerImpl&)streamer).CmdCopyStreamedData(commandBuffer);
  }

  Result DeviceVK::FillFunctionTable(StreamerInterface &table) const {
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

  static Result CreateSwapChain(Device &device, const SwapChainDesc &swapChainDesc, SwapChain *&swapChain) {
    return ((DeviceVK&)device).CreateImplementation<SwapChainVK>(swapChain, swapChainDesc);
  }

  static void DestroySwapChain(SwapChain *swapChain) {
    Destroy((SwapChainVK*)swapChain);
  }

  static Texture *const*GetSwapChainTextures(const SwapChain &swapChain, uint32_t &textureNum) {
    return ((SwapChainVK&)swapChain).GetTextures(textureNum);
  }

  static Result GetDisplayDesc(SwapChain &swapChain, DisplayDesc &displayDesc) {
    return ((SwapChainVK&)swapChain).GetDisplayDesc(displayDesc);
  }

  static Result AcquireNextTexture(SwapChain &swapChain, Fence &acquireSemaphore, uint32_t &textureIndex) {
    return ((SwapChainVK&)swapChain).AcquireNextTexture((FenceVK&)acquireSemaphore, textureIndex);
  }

  static Result WaitForPresent(SwapChain &swapChain) {
    return ((SwapChainVK&)swapChain).WaitForPresent();
  }

  static Result QueuePresent(SwapChain &swapChain, Fence &releaseSemaphore) {
    return ((SwapChainVK&)swapChain).Present((FenceVK&)releaseSemaphore);
  }

  Result DeviceVK::FillFunctionTable(SwapChainInterface &table) const {
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

  static Result CreateUpscaler(Device &device, const UpscalerDesc &upscalerDesc, Upscaler *&upscaler) {
    DeviceVK &deviceVK = (DeviceVK&)device;
    UpscalerImpl *impl = Allocate<UpscalerImpl>(deviceVK.GetAllocationCallbacks(), device, deviceVK.GetCoreInterface());
    Result result = impl->Create(upscalerDesc);

    if (result != Result::Success) {
      Destroy(impl);
      upscaler = nullptr;
    }
    else
      upscaler = (Upscaler*)impl;

    return result;
  }

  static void DestroyUpscaler(Upscaler *upscaler) {
    Destroy((UpscalerImpl*)upscaler);
  }

  static bool IsUpscalerSupported(const Device &device, UpscalerType upscalerType) {
    DeviceVK &deviceVK = (DeviceVK&)device;

    return IsUpscalerSupported(deviceVK.GetDesc(), upscalerType);
  }

  static void GetUpscalerProps(const Upscaler &upscaler, UpscalerProps &upscalerProps) {
    UpscalerImpl &upscalerVK = (UpscalerImpl&)upscaler;

    return upscalerVK.GetUpscalerProps(upscalerProps);
  }

  static void CmdDispatchUpscale(CommandBuffer &commandBuffer, Upscaler &upscaler,
                                 const DispatchUpscaleDesc &dispatchUpscalerDesc) {
    UpscalerImpl &upscalerVK = (UpscalerImpl&)upscaler;

    upscalerVK.CmdDispatchUpscale(commandBuffer, dispatchUpscalerDesc);
  }

  Result DeviceVK::FillFunctionTable(UpscalerInterface &table) const {
    table.CreateUpscaler = ::CreateUpscaler;
    table.DestroyUpscaler = ::DestroyUpscaler;
    table.IsUpscalerSupported = ::IsUpscalerSupported;
    table.GetUpscalerProps = ::GetUpscalerProps;
    table.CmdDispatchUpscale = ::CmdDispatchUpscale;

    return Result::Success;
  }

#pragma endregion

  //============================================================================================================================================================================================
#pragma region[  WrapperVK  ]

  static Result CreateCommandAllocatorVK(Device &device, const CommandAllocatorVKDesc &commandAllocatorVKDesc,
                                         CommandAllocator *&commandAllocator) {
    return ((DeviceVK&)device).CreateImplementation<CommandAllocatorVK>(commandAllocator, commandAllocatorVKDesc);
  }

  static Result CreateCommandBufferVK(Device &device, const CommandBufferVKDesc &commandBufferVKDesc,
                                      CommandBuffer *&commandBuffer) {
    return ((DeviceVK&)device).CreateImplementation<CommandBufferVK>(commandBuffer, commandBufferVKDesc);
  }

  static Result CreateDescriptorPoolVK(Device &device, const DescriptorPoolVKDesc &descriptorPoolVKDesc,
                                       DescriptorPool *&descriptorPool) {
    return ((DeviceVK&)device).CreateImplementation<DescriptorPoolVK>(descriptorPool, descriptorPoolVKDesc);
  }

  static Result CreateBufferVK(Device &device, const BufferVKDesc &bufferVKDesc, Buffer *&buffer) {
    return ((DeviceVK&)device).CreateImplementation<BufferVK>(buffer, bufferVKDesc);
  }

  static Result CreateTextureVK(Device &device, const TextureVKDesc &textureVKDesc, Texture *&texture) {
    return ((DeviceVK&)device).CreateImplementation<TextureVK>(texture, textureVKDesc);
  }

  static Result CreateMemoryVK(Device &device, const MemoryVKDesc &memoryVKDesc, Memory *&memory) {
    return ((DeviceVK&)device).CreateImplementation<MemoryVK>(memory, memoryVKDesc);
  }

  static Result CreatePipelineVK(Device &device, const PipelineVKDesc &pipelineVKDesc, Pipeline *&pipeline) {
    return ((DeviceVK&)device).CreateImplementation<PipelineVK>(pipeline, pipelineVKDesc);
  }

  static Result CreateQueryPoolVK(Device &device, const QueryPoolVKDesc &queryPoolVKDesc, QueryPool *&queryPool) {
    return ((DeviceVK&)device).CreateImplementation<QueryPoolVK>(queryPool, queryPoolVKDesc);
  }

  static Result CreateFenceVK(Device &device, const FenceVKDesc &fenceVKDesc, Fence *&fence) {
    return ((DeviceVK&)device).CreateImplementation<FenceVK>(fence, fenceVKDesc);
  }

  static Result CreateAccelerationStructureVK(Device &device,
                                              const AccelerationStructureVKDesc &accelerationStructureVKDesc,
                                              AccelerationStructure *&accelerationStructure) {
    return ((DeviceVK&)device).CreateImplementation<AccelerationStructureVK>(
      accelerationStructure, accelerationStructureVKDesc);
  }

  static uint32_t GetQueueFamilyIndexVK(const Queue &queue) {
    return ((QueueVK&)queue).GetFamilyIndex();
  }

  static VKHandle GetPhysicalDeviceVK(const Device &device) {
    return (VkPhysicalDevice)((DeviceVK&)device);
  }

  static VKHandle GetInstanceVK(const Device &device) {
    return (VkInstance)((DeviceVK&)device);
  }

  static void *GetInstanceProcAddrVK(const Device &device) {
    return (void*)((DeviceVK&)device).GetDispatchTable().GetInstanceProcAddr;
  }

  static void *GetDeviceProcAddrVK(const Device &device) {
    return (void*)((DeviceVK&)device).GetDispatchTable().GetDeviceProcAddr;
  }

  Result DeviceVK::FillFunctionTable(WrapperVKInterface &table) const {
    table.CreateCommandAllocatorVK = ::CreateCommandAllocatorVK;
    table.CreateCommandBufferVK = ::CreateCommandBufferVK;
    table.CreateDescriptorPoolVK = ::CreateDescriptorPoolVK;
    table.CreateBufferVK = ::CreateBufferVK;
    table.CreateTextureVK = ::CreateTextureVK;
    table.CreateMemoryVK = ::CreateMemoryVK;
    table.CreatePipelineVK = ::CreatePipelineVK;
    table.CreateQueryPoolVK = ::CreateQueryPoolVK;
    table.CreateFenceVK = ::CreateFenceVK;
    table.CreateAccelerationStructureVK = ::CreateAccelerationStructureVK;
    table.GetQueueFamilyIndexVK = ::GetQueueFamilyIndexVK;
    table.GetPhysicalDeviceVK = ::GetPhysicalDeviceVK;
    table.GetInstanceVK = ::GetInstanceVK;
    table.GetDeviceProcAddrVK = ::GetDeviceProcAddrVK;
    table.GetInstanceProcAddrVK = ::GetInstanceProcAddrVK;

    return Result::Success;
  }

#pragma endregion
}
