#include <algorithm>
#include <core/rhi/extensions/helper.hpp>
#include <core/rhi/rhi.hpp>
#include "shared_external.hpp"
#include "helper_interface.hpp"

namespace {
  // Helper data upload
  constexpr uint32_t BARRIERS_PER_PASS = 256;
  constexpr uint64_t MAX_UPLOAD_BUFFER_SIZE = 64 * 1024 * 1024;

  enum class BarrierMode {
    Initial, // transition to COPY_DEST state
    Final, // transition from COPY_DEST to "final" state
    FinalNoData, // initial state is not needed, since there is nothing to upload
  };
}

namespace Core::RHI {
  static void DoTransition(const CoreInterface &m_iCore, CommandBuffer *commandBuffer, BarrierMode barrierMode,
                           const TextureUploadDesc *textureUploadDescs, uint32_t textureDataDescNum) {
    TextureBarrierDesc textureBarriers[BARRIERS_PER_PASS];

    constexpr AccessLayoutStage copyDestState = {
      AccessBits::CopyDestination, Layout::CopyDestination, StageBits::All
    };
    // we don't know which stages to wait
    constexpr AccessLayoutStage unknownState = {AccessBits::None, Layout::Undefined, StageBits::None};
    // since the whole resource is updated, don't care about the previous state

    for (uint32_t i = 0; i < textureDataDescNum;) {
      uint32_t passEnd = std::min(i + BARRIERS_PER_PASS, textureDataDescNum);

      uint32_t n = 0;
      for (; i < passEnd; i++) {
        const TextureUploadDesc &textureUploadDesc = textureUploadDescs[i];
        const TextureDesc &textureDesc = m_iCore.getTextureDesc(*textureUploadDesc.texture);

        TextureBarrierDesc &barrier = textureBarriers[n];
        barrier = {};
        barrier.texture = textureUploadDesc.texture;
        barrier.mipNum = textureDesc.mipNum;
        barrier.layerNum = textureDesc.layerNum;
        barrier.before = barrierMode == BarrierMode::Final ? copyDestState : unknownState;
        barrier.after = barrierMode == BarrierMode::Initial ? copyDestState : textureUploadDesc.after;

        if (barrierMode != BarrierMode::Initial)
          barrier.planes = textureUploadDesc.planes;

        // Filter out redundant barriers
        if (barrier.before.access != barrier.after.access || barrier.before.layout != barrier.after.layout)
          n++;
      }

      BarrierDesc barrierGroup = {};
      barrierGroup.textures = textureBarriers;
      barrierGroup.textureNum = n;

      m_iCore.cmdBarrier(*commandBuffer, barrierGroup);
    }
  }

  static void DoTransition(const CoreInterface &m_iCore, CommandBuffer *commandBuffer, BarrierMode barrierMode,
                           const BufferUploadDesc *bufferUploadDescs, uint32_t bufferUploadDescNum) {
    BufferBarrierDesc bufferBarriers[BARRIERS_PER_PASS];

    constexpr AccessStage copyDestState = {AccessBits::CopyDestination, StageBits::All};
    // we don't know which stages to wait
    constexpr AccessStage unknownState = {AccessBits::None, StageBits::None};
    // since the whole resource is updated, don't care about the previous state

    for (uint32_t i = 0; i < bufferUploadDescNum;) {
      uint32_t passEnd = std::min(i + BARRIERS_PER_PASS, bufferUploadDescNum);

      uint32_t n = 0;
      for (; i < passEnd; i++) {
        const BufferUploadDesc &bufferUploadDesc = bufferUploadDescs[i];

        BufferBarrierDesc &barrier = bufferBarriers[n];
        barrier = {};
        barrier.buffer = bufferUploadDesc.buffer;
        barrier.before = barrierMode == BarrierMode::Final ? copyDestState : unknownState;
        barrier.after = barrierMode == BarrierMode::Initial ? copyDestState : bufferUploadDesc.after;

        // Filter out redundant barriers
        if (barrier.before.access != barrier.after.access)
          n++;
      }

      BarrierDesc barrierGroup = {};
      barrierGroup.buffers = bufferBarriers;
      barrierGroup.bufferNum = n;

      m_iCore.cmdBarrier(*commandBuffer, barrierGroup);
    }
  }

  Result HelperDataUpload::uploadData(const TextureUploadDesc *textureUploadDescs, uint32_t textureUploadDescNum,
                                      const BufferUploadDesc *bufferUploadDescs, uint32_t bufferUploadDescNum) {
    Result result = create(textureUploadDescs, textureUploadDescNum, bufferUploadDescs, bufferUploadDescNum);

    if (result == Result::Success)
      result = uploadTextures(textureUploadDescs, textureUploadDescNum);
    if (result == Result::Success)
      result = uploadBuffers(bufferUploadDescs, bufferUploadDescNum);

    coreInterface.destroyCommandBuffer(commandBuffer);
    coreInterface.destroyCommandAllocator(commandAllocator);
    coreInterface.destroyFence(fence);
    coreInterface.destroyBuffer(uploadBuffer);

    return result;
  }

  Result HelperDataUpload::create(const TextureUploadDesc *textureUploadDescs, uint32_t textureUploadDescNum,
                                  const BufferUploadDesc *bufferUploadDescs, uint32_t bufferUploadDescNum) {
    const DeviceInfo &DeviceInfo = coreInterface.getDeviceDesc(device);

    {
      // Calculate upload buffer size
      uint64_t maxSubresourceSize = 0;
      uint64_t totalSize = 0;

      for (uint32_t i = 0; i < textureUploadDescNum; i++) {
        const TextureUploadDesc &textureUploadDesc = textureUploadDescs[i];
        if (textureUploadDesc.subresources) {
          const TextureSubresourceUploadDesc &subresource0 = textureUploadDesc.subresources[0];
          const TextureDesc &textureDesc = coreInterface.getTextureDesc(*textureUploadDesc.texture);

          uint32_t sliceRowNum = subresource0.slicePitch / subresource0.rowPitch;
          uint64_t alignedRowPitch = Align(subresource0.rowPitch, DeviceInfo.memoryAlignment.uploadBufferTextureRow);
          uint64_t alignedSlicePitch = Align(sliceRowNum * alignedRowPitch,
                                             DeviceInfo.memoryAlignment.uploadBufferTextureSlice);
          uint64_t alignedSize = alignedSlicePitch * subresource0.sliceNum;

          NRI_CHECK(alignedSize != 0, "Unexpected");

          maxSubresourceSize = std::max(maxSubresourceSize, alignedSize);

          alignedSize *= textureDesc.layerNum;
          if (textureDesc.mipNum > 1)
            totalSize += (alignedSize * 4) / 3; // assume full mip chain
          else
            totalSize += alignedSize;
        }
      }

      for (uint32_t i = 0; i < bufferUploadDescNum; i++) {
        // Doesn't contribute to "maxSubresourceSize" because buffer copies can work with any non-0 upload buffer size
        const BufferUploadDesc &bufferUploadDesc = bufferUploadDescs[i];
        if (bufferUploadDesc.data) {
          const BufferDesc &bufferDesc = coreInterface.getBufferDesc(*bufferUploadDesc.buffer);

          totalSize += bufferDesc.size;
        }
      }

      // Can use up to "MAX_UPLOAD_BUFFER_SIZE" bytes
      uploadBufferSize = std::min(totalSize, MAX_UPLOAD_BUFFER_SIZE);

      // Worst case subresource must fit
      uploadBufferSize = std::max(uploadBufferSize, maxSubresourceSize);
    }

    // Create upload buffer
    if (uploadBufferSize) {
      BufferDesc bufferDesc = {};
      bufferDesc.size = uploadBufferSize;

      Result result = coreInterface.createCommittedBuffer(device, MemoryLocation::HostUpload, 0.0f, bufferDesc,
                                                    uploadBuffer);
      if (result != Result::Success)
        return result;
    }

    {
      // Create other resources
      Result result = coreInterface.createFence(device, 0, fence);
      if (result != Result::Success)
        return result;

      result = coreInterface.createCommandAllocator(queue, commandAllocator);
      if (result != Result::Success)
        return result;

      result = coreInterface.createCommandBuffer(*commandAllocator, commandBuffer);
      if (result != Result::Success)
        return result;
    }

    return Result::Success;
  }

  Result HelperDataUpload::uploadTextures(const TextureUploadDesc *textureUploadDescs, uint32_t textureDataDescNum) {
    if (!textureDataDescNum)
      return Result::Success;

    uint32_t i = 0;
    for (; i < textureDataDescNum; i++) {
      const TextureUploadDesc &textureUploadDesc = textureUploadDescs[i];
      if (textureUploadDesc.subresources)
        break;
    }

    BarrierMode barrierMode = i == textureDataDescNum ? BarrierMode::FinalNoData : BarrierMode::Final;

    bool isInitial = true;
    Dim_t layerOffset = 0;
    Dim_t mipOffset = 0;
    i = 0;

    while (i < textureDataDescNum) {
      if (!isInitial) {
        Result result = EndCommandBuffersAndSubmit();
        if (result != Result::Success)
          return result;
      }

      Result result = coreInterface.beginCommandBuffer(*commandBuffer, nullptr);
      if (result != Result::Success)
        return result;

      if (isInitial) {
        if (barrierMode != BarrierMode::FinalNoData)
          DoTransition(coreInterface, commandBuffer, BarrierMode::Initial, textureUploadDescs, textureDataDescNum);
        isInitial = false;
      }

      uploadBufferOffset = 0;
      for (; i < textureDataDescNum && CopyTextureContent(textureUploadDescs[i], layerOffset, mipOffset); i++);
    }

    DoTransition(coreInterface, commandBuffer, barrierMode, textureUploadDescs, textureDataDescNum);

    return EndCommandBuffersAndSubmit();
  }

  Result HelperDataUpload::uploadBuffers(const BufferUploadDesc *bufferUploadDescs, uint32_t bufferUploadDescNum) {
    if (!bufferUploadDescNum)
      return Result::Success;

    uint32_t i = 0;
    for (; i < bufferUploadDescNum; i++) {
      const BufferUploadDesc &bufferUploadDesc = bufferUploadDescs[i];
      if (bufferUploadDesc.data)
        break;
    }

    BarrierMode barrierMode = i == bufferUploadDescNum ? BarrierMode::FinalNoData : BarrierMode::Final;

    bool isInitial = true;
    uint64_t bufferContentOffset = 0;
    i = 0;

    while (i < bufferUploadDescNum) {
      if (!isInitial) {
        Result result = EndCommandBuffersAndSubmit();
        if (result != Result::Success)
          return result;
      }

      Result result = coreInterface.beginCommandBuffer(*commandBuffer, nullptr);
      if (result != Result::Success)
        return result;

      if (isInitial) {
        if (barrierMode != BarrierMode::FinalNoData)
          DoTransition(coreInterface, commandBuffer, BarrierMode::Initial, bufferUploadDescs, bufferUploadDescNum);
        isInitial = false;
      }

      uploadBufferOffset = 0;
      mappedMemory = (uint8_t*)coreInterface.mapBuffer(*uploadBuffer, 0, uploadBufferSize);

      for (; i < bufferUploadDescNum && CopyBufferContent(bufferUploadDescs[i], bufferContentOffset); i++);

      coreInterface.unmapBuffer(*uploadBuffer);
    }

    DoTransition(coreInterface, commandBuffer, barrierMode, bufferUploadDescs, bufferUploadDescNum);

    return EndCommandBuffersAndSubmit();
  }

  Result HelperDataUpload::EndCommandBuffersAndSubmit() {
    Result result = coreInterface.endCommandBuffer(*commandBuffer);

    if (result == Result::Success) {
      FenceSubmitDesc fenceSubmitDesc = {};
      fenceSubmitDesc.fence = fence;
      fenceSubmitDesc.value = fenceValue;

      QueueSubmitDesc queueSubmitDesc = {};
      queueSubmitDesc.commandBufferNum = 1;
      queueSubmitDesc.commandBuffers = &commandBuffer;
      queueSubmitDesc.signalFences = &fenceSubmitDesc;
      queueSubmitDesc.signalFenceNum = 1;

      result = coreInterface.queueSubmit(queue, queueSubmitDesc);
      if (result == Result::Success) {
        coreInterface.wait(*fence, fenceValue);
        coreInterface.resetCommandAllocator(*commandAllocator);

        fenceValue++;
      }
    }

    return result;
  }

  bool HelperDataUpload::CopyTextureContent(const TextureUploadDesc &textureUploadDesc, Dim_t &layerOffset,
                                            Dim_t &mipOffset) {
    if (!textureUploadDesc.subresources)
      return true;

    const DeviceInfo &DeviceInfo = coreInterface.getDeviceDesc(device);
    const TextureDesc &textureDesc = coreInterface.getTextureDesc(*textureUploadDesc.texture);

    for (; layerOffset < textureDesc.layerNum; layerOffset++) {
      for (; mipOffset < textureDesc.mipNum; mipOffset++) {
        const auto &subresource = textureUploadDesc.subresources[layerOffset * textureDesc.mipNum + mipOffset];

        uint32_t sliceRowNum = subresource.slicePitch / subresource.rowPitch;
        uint32_t alignedRowPitch = Align(subresource.rowPitch, DeviceInfo.memoryAlignment.uploadBufferTextureRow);
        uint32_t alignedSlicePitch = Align(sliceRowNum * alignedRowPitch,
                                           DeviceInfo.memoryAlignment.uploadBufferTextureSlice);
        uint64_t alignedSize = uint64_t(alignedSlicePitch) * subresource.sliceNum;
        uint64_t freeSpace = uploadBufferSize - uploadBufferOffset;

        if (alignedSize > freeSpace) {
          NRI_CHECK(alignedSize <= uploadBufferSize, "Unexpected");
          return false;
        }

        // Upload data (D3D11 does not allow to use upload buffer while it's mapped)
        uint8_t *slices = (uint8_t*)coreInterface.mapBuffer(*uploadBuffer, uploadBufferOffset,
                                                      subresource.sliceNum * alignedSlicePitch);
        {
          for (uint32_t k = 0; k < subresource.sliceNum; k++) {
            for (uint32_t l = 0; l < sliceRowNum; l++) {
              uint8_t *dstRow = slices + k * alignedSlicePitch + l * alignedRowPitch;
              uint8_t *srcRow = (uint8_t*)subresource.slices + k * subresource.slicePitch + l * subresource.rowPitch;
              memcpy(dstRow, srcRow, subresource.rowPitch);
            }
          }
        }
        coreInterface.unmapBuffer(*uploadBuffer);

        {
          // Copy
          TextureDataLayoutDesc srcDataLayout = {};
          srcDataLayout.offset = uploadBufferOffset;
          srcDataLayout.rowPitch = alignedRowPitch;
          srcDataLayout.slicePitch = alignedSlicePitch;

          TextureRegionDesc dstRegion = {};
          dstRegion.layerOffset = layerOffset;
          dstRegion.mipOffset = mipOffset;

          coreInterface.cmdUploadBufferToTexture(*commandBuffer, *textureUploadDesc.texture, dstRegion, *uploadBuffer,
                                           srcDataLayout);
        }

        // Increment buffer offset
        uploadBufferOffset += alignedSize;
      }
      mipOffset = 0;
    }
    layerOffset = 0;

    return true;
  }

  bool HelperDataUpload::CopyBufferContent(const BufferUploadDesc &bufferUploadDesc, uint64_t &bufferContentOffset) {
    if (!bufferUploadDesc.data)
      return true;

    const BufferDesc &bufferDesc = coreInterface.getBufferDesc(*bufferUploadDesc.buffer);

    uint64_t freeSpace = uploadBufferSize - uploadBufferOffset;
    uint64_t copySize = std::min(bufferDesc.size - bufferContentOffset, freeSpace);

    if (freeSpace == 0)
      return false;

    memcpy(mappedMemory + uploadBufferOffset, (uint8_t*)bufferUploadDesc.data + bufferContentOffset, copySize);

    coreInterface.cmdCopyBuffer(*commandBuffer, *bufferUploadDesc.buffer, bufferContentOffset, *uploadBuffer,
                          uploadBufferOffset, copySize);

    bufferContentOffset += copySize;
    uploadBufferOffset += copySize;

    if (bufferContentOffset != bufferDesc.size)
      return false;

    bufferContentOffset = 0;

    return true;
  }

  // HelperDeviceMemoryAllocator
  HelperDeviceMemoryAllocator::MemoryHeap::MemoryHeap(MemoryType memoryType, const StdAllocator<uint8_t> &stdAllocator)
    : buffers(stdAllocator)
      , bufferOffsets(stdAllocator)
      , textures(stdAllocator)
      , textureOffsets(stdAllocator)
      , size(0)
      , type(memoryType) {
  }

  HelperDeviceMemoryAllocator::HelperDeviceMemoryAllocator(const CoreInterface &NRI, Device &device)
    : m_iCore(NRI)
      , m_Device(device)
      , m_Heaps(((DeviceBase&)device).getStdAllocator())
      , m_DedicatedBuffers(((DeviceBase&)device).getStdAllocator())
      , m_DedicatedTextures(((DeviceBase&)device).getStdAllocator())
      , m_BufferBindingDescs(((DeviceBase&)device).getStdAllocator())
      , m_TextureBindingDescs(((DeviceBase&)device).getStdAllocator()) {
  }

  uint32_t HelperDeviceMemoryAllocator::CalculateAllocationNumber(const ResourceGroupDesc &resourceGroupDesc) {
    GroupByMemoryType(resourceGroupDesc.memoryLocation, resourceGroupDesc);

    size_t allocationNum = m_Heaps.size() + m_DedicatedBuffers.size() + m_DedicatedTextures.size();

    return (uint32_t)allocationNum;
  }

  Result HelperDeviceMemoryAllocator::AllocateAndBindMemory(const ResourceGroupDesc &resourceGroupDesc,
                                                            Memory **allocations) {
    size_t allocationNum = 0;
    Result result = TryToAllocateAndBindMemory(resourceGroupDesc, allocations, allocationNum);

    if (result != Result::Success) {
      for (size_t i = 0; i < allocationNum; i++) {
        m_iCore.freeMemory(allocations[i]);
        allocations[i] = nullptr;
      }
    }

    return result;
  }

  Result HelperDeviceMemoryAllocator::TryToAllocateAndBindMemory(const ResourceGroupDesc &resourceGroupDesc,
                                                                 Memory **allocations, size_t &allocationNum) {
    GroupByMemoryType(resourceGroupDesc.memoryLocation, resourceGroupDesc);

    for (MemoryHeap &heap

         :
         m_Heaps
    ) {
      Memory *&memory = allocations[allocationNum];

      bool hasMultisampleTextures = false;
      for (Texture *texture : heap.textures) {
        const TextureDesc &textureDesc = m_iCore.getTextureDesc(*texture);
        if (textureDesc.sampleNum > 1) {
          hasMultisampleTextures = true;
          break;
        }
      }

      AllocateMemoryDesc allocateMemoryDesc = {};
      allocateMemoryDesc.type = heap.type;
      allocateMemoryDesc.size = heap.size;
      allocateMemoryDesc.allowMultisampleTextures = hasMultisampleTextures;
      allocateMemoryDesc.priority = resourceGroupDesc.residencyPriority;

      Result result = m_iCore.allocateMemory(device, allocateMemoryDesc, memory);
      if (result != Result::Success)
        return result;

      FillMemoryBindingDescs(heap.buffers.data(), heap.bufferOffsets.data(), (uint32_t)heap.buffers.size(), *memory);
      FillMemoryBindingDescs(heap.textures.data(), heap.textureOffsets.data(), (uint32_t)heap.textures.size(), *memory);

      allocationNum++;
    }

    Result result = ProcessDedicatedResources(resourceGroupDesc, allocations, allocationNum);
    if (result != Result::Success)
      return result;

    result = m_iCore.bindBufferMemory(m_BufferBindingDescs.data(), (uint32_t)m_BufferBindingDescs.size());
    if (result != Result::Success)
      return result;

    result = m_iCore.bindTextureMemory(m_TextureBindingDescs.data(), (uint32_t)m_TextureBindingDescs.size());

    return result;
  }

  Result HelperDeviceMemoryAllocator::ProcessDedicatedResources(const ResourceGroupDesc &resourceGroupDesc,
                                                                Memory **allocations, size_t &allocationNum) {
    constexpr uint64_t zeroOffset = 0;
    MemoryDesc memoryDesc = {};

    for (size_t i = 0; i < m_DedicatedBuffers.size(); i++) {
      m_iCore.getBufferMemoryDesc(*m_DedicatedBuffers[i], resourceGroupDesc.memoryLocation, memoryDesc);

      Memory *&memory = allocations[allocationNum];

      AllocateMemoryDesc allocateMemoryDesc = {};
      allocateMemoryDesc.type = memoryDesc.type;
      allocateMemoryDesc.size = memoryDesc.size;
      allocateMemoryDesc.priority = resourceGroupDesc.residencyPriority;

      Result result = m_iCore.allocateMemory(device, allocateMemoryDesc, memory);
      if (result != Result::Success)
        return result;

      FillMemoryBindingDescs(m_DedicatedBuffers.data() + i, &zeroOffset, 1, *memory);

      allocationNum++;
    }

    for (size_t i = 0; i < m_DedicatedTextures.size(); i++) {
      m_iCore.getTextureMemoryDesc(*m_DedicatedTextures[i], resourceGroupDesc.memoryLocation, memoryDesc);

      Memory *&memory = allocations[allocationNum];

      AllocateMemoryDesc allocateMemoryDesc = {};
      allocateMemoryDesc.type = memoryDesc.type;
      allocateMemoryDesc.size = memoryDesc.size;
      allocateMemoryDesc.priority = resourceGroupDesc.residencyPriority;

      Result result = m_iCore.allocateMemory(device, allocateMemoryDesc, memory);
      if (result != Result::Success)
        return result;

      FillMemoryBindingDescs(m_DedicatedTextures.data() + i, &zeroOffset, 1, *memory);

      allocationNum++;
    }

    return Result::Success;
  }

  HelperDeviceMemoryAllocator::MemoryHeap &HelperDeviceMemoryAllocator::FindOrCreateHeap(
    const MemoryDesc &memoryDesc, uint64_t preferredMemorySize) {
    if (preferredMemorySize == 0)
      preferredMemorySize = 256 * 1024 * 1024;

    for (MemoryHeap &heap

         :
         m_Heaps
    ) {
      uint64_t offset = Align(heap.size, memoryDesc.alignment);
      uint64_t newSize = offset + memoryDesc.size;

      if (heap.type == memoryDesc.type && newSize <= preferredMemorySize)
        return heap;
    }

    m_Heaps.push_back(MemoryHeap(memoryDesc.type, ((DeviceBase&)device).getStdAllocator()));

    return m_Heaps.back();
  }

  void HelperDeviceMemoryAllocator::GroupByMemoryType(MemoryLocation memoryLocation,
                                                      const ResourceGroupDesc &resourceGroupDesc) {
    struct BufferAndMemoryDesc {
      Buffer *buffer;
      MemoryDesc memoryDesc;
    };

    struct TextureAndMemoryDesc {
      Texture *texture;
      MemoryDesc memoryDesc;
    };

    // Copy to temp memory
    Scratch<BufferAndMemoryDesc> buffers = NRI_ALLOCATE_SCRATCH((DeviceBase&)device, BufferAndMemoryDesc,
                                                                resourceGroupDesc.bufferNum);
    for (uint32_t i = 0; i < resourceGroupDesc.bufferNum; i++) {
      Buffer *buffer = resourceGroupDesc.buffers[i];

      buffers[i].buffer = buffer;
      m_iCore.getBufferMemoryDesc(*buffer, memoryLocation, buffers[i].memoryDesc);
    }

    Scratch<TextureAndMemoryDesc> textures = NRI_ALLOCATE_SCRATCH((DeviceBase&)device, TextureAndMemoryDesc,
                                                                  resourceGroupDesc.textureNum);
    for (uint32_t i = 0; i < resourceGroupDesc.textureNum; i++) {
      Texture *texture = resourceGroupDesc.textures[i];

      textures[i].texture = texture;
      m_iCore.getTextureMemoryDesc(*texture, memoryLocation, textures[i].memoryDesc);
    }

    // Sort by "alignment"
    if (resourceGroupDesc.bufferNum > 1) {
      std::sort(&buffers[0], &buffers[resourceGroupDesc.bufferNum - 1],
                [](const BufferAndMemoryDesc &a, const BufferAndMemoryDesc &b) -> bool {
                  // Primary key: group by type
                  if (a.memoryDesc.type != b.memoryDesc.type)
                    return a.memoryDesc.type < b.memoryDesc.type;

                  // Secondary key: smallest to largest alignment
                  return a.memoryDesc.alignment < b.memoryDesc.alignment;
                });
    }

    if (resourceGroupDesc.textureNum > 1) {
      std::sort(&textures[0], &textures[resourceGroupDesc.textureNum - 1],
                [](const TextureAndMemoryDesc &a, const TextureAndMemoryDesc &b) -> bool {
                  // Primary key: group by type
                  if (a.memoryDesc.type != b.memoryDesc.type)
                    return a.memoryDesc.type < b.memoryDesc.type;

                  // Secondary key: smallest to largest alignment
                  return a.memoryDesc.alignment < b.memoryDesc.alignment;
                });
    }

    // Linearly assign memory
    for (uint32_t i = 0; i < resourceGroupDesc.bufferNum; i++) {
      Buffer *buffer = buffers[i].buffer;
      const MemoryDesc &memoryDesc = buffers[i].memoryDesc;

      if (memoryDesc.mustBeDedicated)
        m_DedicatedBuffers.push_back(buffer);
      else {
        MemoryHeap &heap = FindOrCreateHeap(memoryDesc, resourceGroupDesc.preferredMemorySize);

        uint64_t offset = Align(heap.size, memoryDesc.alignment);

        heap.buffers.push_back(buffer);
        heap.bufferOffsets.push_back(offset);
        heap.size = offset + memoryDesc.size;
      }
    }

    for (uint32_t i = 0; i < resourceGroupDesc.textureNum; i++) {
      Texture *texture = textures[i].texture;
      const MemoryDesc &memoryDesc = textures[i].memoryDesc;

      if (memoryDesc.mustBeDedicated)
        m_DedicatedTextures.push_back(texture);
      else {
        MemoryHeap &heap = FindOrCreateHeap(memoryDesc, resourceGroupDesc.preferredMemorySize);

        if (heap.textures.empty()) {
          const DeviceInfo &DeviceInfo = m_iCore.getDeviceDesc(device);
          heap.size = Align(heap.size, DeviceInfo.memory.bufferTextureGranularity);
        }

        uint64_t offset = Align(heap.size, memoryDesc.alignment);

        heap.textures.push_back(texture);
        heap.textureOffsets.push_back(offset);
        heap.size = offset + memoryDesc.size;
      }
    }
  }

  void HelperDeviceMemoryAllocator::FillMemoryBindingDescs(Buffer *const *buffers, const uint64_t *bufferOffsets,
                                                           uint32_t bufferNum, Memory &memory) {
    for (uint32_t i = 0; i < bufferNum; i++) {
      BindBufferMemoryDesc desc = {};
      desc.memory = &memory;
      desc.buffer = buffers[i];
      desc.offset = bufferOffsets[i];

      m_BufferBindingDescs.push_back(desc);
    }
  }

  void HelperDeviceMemoryAllocator::FillMemoryBindingDescs(Texture *const *textures, const uint64_t *textureOffsets,
                                                           uint32_t textureNum, Memory &memory) {
    for (uint32_t i = 0; i < textureNum; i++) {
      BindTextureMemoryDesc desc = {};
      desc.memory = &memory;
      desc.texture = textures[i];
      desc.offset = textureOffsets[i];

      m_TextureBindingDescs.push_back(desc);
    }
  }
}
