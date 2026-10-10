// © 2021 NVIDIA Corporation

#include "../shared/SharedExternal.hpp"

using namespace Core::RHI;

template <typename T>
constexpr T *DummyObject() {
  return (T*)(size_t)(1);
}

struct DeviceNONE final: public DeviceBase {
  inline DeviceNONE(const CallbackInterface &callbacks, const AllocationCallbacks &allocationCallbacks,
                    const AdapterDesc *adapterDesc)
    : DeviceBase(callbacks, allocationCallbacks) {
    if (adapterDesc)
      m_Desc.adapterDesc = *adapterDesc;

    for (uint32_t i = 0; i < (uint32_t)QueueType::Count; i++)
      m_Desc.adapterDesc.queueNum[i] = 4;

    m_Desc.graphicsBackend = GraphicsBackend::None;
    // m_Desc.nriVersion = NRI_VERSION;
    // m_Desc.shaderModel = NriShaderModel(6, 10);

    m_Desc.viewport.maxNum = 16;
    m_Desc.viewport.boundsMin = -32768;
    m_Desc.viewport.boundsMax = 32767;

    m_Desc.dimensions.attachmentMaxDim = 16384;
    m_Desc.dimensions.attachmentLayerMaxNum = 2048;
    m_Desc.dimensions.texture1DMaxDim = 16384;
    m_Desc.dimensions.texture2DMaxDim = 16384;
    m_Desc.dimensions.texture3DMaxDim = 16384;
    m_Desc.dimensions.textureLayerMaxNum = 16384;
    m_Desc.dimensions.typedBufferMaxDim = uint32_t(-1);

    m_Desc.precision.viewportBits = 8;
    m_Desc.precision.subPixelBits = 8;
    m_Desc.precision.subTexelBits = 8;
    m_Desc.precision.mipmapBits = 8;

    m_Desc.memory.deviceUploadHeapSize = 256 * 1024 * 1024;
    m_Desc.memory.allocationMaxNum = uint32_t(-1);
    m_Desc.memory.samplerAllocationMaxNum = 4096;
    m_Desc.memory.constantBufferMaxRange = 64 * 1024;
    m_Desc.memory.storageBufferMaxRange = uint32_t(-1);
    m_Desc.memory.bufferTextureGranularity = 1;
    m_Desc.memory.bufferMaxSize = uint32_t(-1);

    m_Desc.memoryAlignment.uploadBufferTextureRow = 1;
    m_Desc.memoryAlignment.uploadBufferTextureSlice = 1;
    m_Desc.memoryAlignment.bufferShaderResourceOffset = 1;
    m_Desc.memoryAlignment.constantBufferOffset = 1;
    m_Desc.memoryAlignment.scratchBufferOffset = 1;
    m_Desc.memoryAlignment.shaderBindingTable = 1;
    m_Desc.memoryAlignment.accelerationStructureOffset = 1;
    m_Desc.memoryAlignment.micromapOffset = 1;

    m_Desc.pipelineLayout.descriptorSetMaxNum = 64;
    m_Desc.pipelineLayout.rootConstantMaxSize = 256;
    m_Desc.pipelineLayout.rootDescriptorMaxNum = 64;

    m_Desc.descriptorSet.samplerMaxNum = 1000000;
    m_Desc.descriptorSet.constantBufferMaxNum = 1000000;
    m_Desc.descriptorSet.storageBufferMaxNum = 1000000;
    m_Desc.descriptorSet.textureMaxNum = 1000000;
    m_Desc.descriptorSet.storageTextureMaxNum = 1000000;

    m_Desc.descriptorSet.updateAfterSet.samplerMaxNum = m_Desc.descriptorSet.samplerMaxNum;
    m_Desc.descriptorSet.updateAfterSet.constantBufferMaxNum = m_Desc.descriptorSet.constantBufferMaxNum;
    m_Desc.descriptorSet.updateAfterSet.storageBufferMaxNum = m_Desc.descriptorSet.storageBufferMaxNum;
    m_Desc.descriptorSet.updateAfterSet.textureMaxNum = m_Desc.descriptorSet.textureMaxNum;
    m_Desc.descriptorSet.updateAfterSet.storageTextureMaxNum = m_Desc.descriptorSet.storageTextureMaxNum;

    m_Desc.shaderStage.descriptorSamplerMaxNum = 1000000;
    m_Desc.shaderStage.descriptorConstantBufferMaxNum = 1000000;
    m_Desc.shaderStage.descriptorStorageBufferMaxNum = 1000000;
    m_Desc.shaderStage.descriptorTextureMaxNum = 1000000;
    m_Desc.shaderStage.descriptorStorageTextureMaxNum = 1000000;
    m_Desc.shaderStage.resourceMaxNum = 1000000;

    m_Desc.shaderStage.updateAfterSet.descriptorSamplerMaxNum = m_Desc.shaderStage.descriptorSamplerMaxNum;
    m_Desc.shaderStage.updateAfterSet.descriptorConstantBufferMaxNum = m_Desc.shaderStage.
                                                                              descriptorConstantBufferMaxNum;
    m_Desc.shaderStage.updateAfterSet.descriptorStorageBufferMaxNum = m_Desc.shaderStage.descriptorStorageBufferMaxNum;
    m_Desc.shaderStage.updateAfterSet.descriptorTextureMaxNum = m_Desc.shaderStage.descriptorTextureMaxNum;
    m_Desc.shaderStage.updateAfterSet.descriptorStorageTextureMaxNum = m_Desc.shaderStage.
                                                                              descriptorStorageTextureMaxNum;
    m_Desc.shaderStage.updateAfterSet.resourceMaxNum = m_Desc.shaderStage.resourceMaxNum;

    m_Desc.shaderStage.vertex.attributeMaxNum = 32;
    m_Desc.shaderStage.vertex.streamMaxNum = 32;
    m_Desc.shaderStage.vertex.outputComponentMaxNum = 128;

    m_Desc.shaderStage.tesselationControl.generationMaxLevel = 64.0f;
    m_Desc.shaderStage.tesselationControl.patchPointMaxNum = 32;
    m_Desc.shaderStage.tesselationControl.perVertexInputComponentMaxNum = 128;
    m_Desc.shaderStage.tesselationControl.perVertexOutputComponentMaxNum = 128;
    m_Desc.shaderStage.tesselationControl.perPatchOutputComponentMaxNum = 128;
    m_Desc.shaderStage.tesselationControl.totalOutputComponentMaxNum = 1000000;

    m_Desc.shaderStage.tesselationEvaluation.inputComponentMaxNum = 128;
    m_Desc.shaderStage.tesselationEvaluation.outputComponentMaxNum = 128;

    m_Desc.shaderStage.geometry.invocationMaxNum = 32;
    m_Desc.shaderStage.geometry.inputComponentMaxNum = 128;
    m_Desc.shaderStage.geometry.outputComponentMaxNum = 128;
    m_Desc.shaderStage.geometry.outputVertexMaxNum = 1024;
    m_Desc.shaderStage.geometry.totalOutputComponentMaxNum = 1024;

    m_Desc.shaderStage.fragment.inputComponentMaxNum = 128;
    m_Desc.shaderStage.fragment.attachmentMaxNum = 8;
    m_Desc.shaderStage.fragment.dualSourceAttachmentMaxNum = 1;

    m_Desc.shaderStage.compute.dispatchMaxDim[0] = (uint32_t)(-1);
    m_Desc.shaderStage.compute.dispatchMaxDim[1] = (uint32_t)(-1);
    m_Desc.shaderStage.compute.dispatchMaxDim[2] = (uint32_t)(-1);
    m_Desc.shaderStage.compute.workGroupInvocationMaxNum = (uint32_t)(-1);
    m_Desc.shaderStage.compute.workGroupMaxDim[0] = (uint32_t)(-1);
    m_Desc.shaderStage.compute.workGroupMaxDim[1] = (uint32_t)(-1);
    m_Desc.shaderStage.compute.workGroupMaxDim[2] = (uint32_t)(-1);
    m_Desc.shaderStage.compute.sharedMemoryMaxSize = (uint32_t)(-1);

    m_Desc.shaderStage.task.dispatchWorkGroupMaxNum = (uint32_t)(-1);
    m_Desc.shaderStage.task.dispatchMaxDim[0] = (uint32_t)(-1);
    m_Desc.shaderStage.task.dispatchMaxDim[1] = (uint32_t)(-1);
    m_Desc.shaderStage.task.dispatchMaxDim[2] = (uint32_t)(-1);
    m_Desc.shaderStage.task.workGroupInvocationMaxNum = (uint32_t)(-1);
    m_Desc.shaderStage.task.workGroupMaxDim[0] = (uint32_t)(-1);
    m_Desc.shaderStage.task.workGroupMaxDim[1] = (uint32_t)(-1);
    m_Desc.shaderStage.task.workGroupMaxDim[2] = (uint32_t)(-1);
    m_Desc.shaderStage.task.sharedMemoryMaxSize = (uint32_t)(-1);
    m_Desc.shaderStage.task.payloadMaxSize = (uint32_t)(-1);

    m_Desc.shaderStage.mesh.dispatchWorkGroupMaxNum = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.dispatchMaxDim[0] = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.dispatchMaxDim[1] = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.dispatchMaxDim[2] = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.workGroupInvocationMaxNum = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.workGroupMaxDim[0] = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.workGroupMaxDim[1] = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.workGroupMaxDim[2] = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.sharedMemoryMaxSize = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.outputVerticesMaxNum = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.outputPrimitiveMaxNum = (uint32_t)(-1);
    m_Desc.shaderStage.mesh.outputComponentMaxNum = (uint32_t)(-1);

    m_Desc.shaderStage.rayTracing.shaderGroupIdentifierSize = 32;
    m_Desc.shaderStage.rayTracing.shaderBindingTableMaxStride = (uint32_t)(-1);
    m_Desc.shaderStage.rayTracing.recursionMaxDepth = 31;

    m_Desc.accelerationStructure.primitiveMaxNum = (uint32_t)(-1);
    m_Desc.accelerationStructure.geometryMaxNum = (uint32_t)(-1);
    m_Desc.accelerationStructure.instanceMaxNum = (uint32_t)(-1);
    m_Desc.accelerationStructure.micromapSubdivisionMaxLevel = 12;

    m_Desc.wave.laneMinNum = 32;
    m_Desc.wave.laneMaxNum = 32;
    m_Desc.wave.waveOpsStages = StageBits::AllShaders;
    m_Desc.wave.derivativeOpsStages = StageBits::AllShaders;
    m_Desc.wave.quadOpsStages = StageBits::AllShaders;

    m_Desc.other.timestampFrequencyHz = 1;
    m_Desc.other.drawIndirectMaxNum = uint32_t(-1);
    m_Desc.other.samplerLodBiasMax = 16.0f;
    m_Desc.other.samplerAnisotropyMax = 16;
    m_Desc.other.texelOffsetMin = -8;
    m_Desc.other.texelOffsetMax = 7;
    m_Desc.other.texelGatherOffsetMin = -8;
    m_Desc.other.texelGatherOffsetMax = 7;
    m_Desc.other.clipDistanceMaxNum = 8;
    m_Desc.other.cullDistanceMaxNum = 8;
    m_Desc.other.combinedClipAndCullDistanceMaxNum = 8;
    m_Desc.other.viewMaxNum = 4;
    m_Desc.other.shadingRateAttachmentTileSize = 16;

    memset(&m_Desc.tiers, 0xFF, sizeof(m_Desc.tiers));
    memset(&m_Desc.features, 1, sizeof(m_Desc.features));
    memset(&m_Desc.shaderFeatures, 1, sizeof(m_Desc.shaderFeatures));
  }

  inline ~DeviceNONE() {
  }

  //================================================================================================================
  // DeviceBase
  //================================================================================================================

  inline const DeviceInfo &GetDesc() const override {
    return m_Desc;
  }

  inline void Destruct() override {
    Destroy(GetAllocationCallbacks(), this);
  }

  Result FillFunctionTable(CoreInterface &table) const override;
  Result FillFunctionTable(HelperInterface &table) const override;
  Result FillFunctionTable(LowLatencyInterface &table) const override;
  Result FillFunctionTable(MeshShaderInterface &table) const override;
  Result FillFunctionTable(RayTracingInterface &table) const override;
  Result FillFunctionTable(StreamerInterface &table) const override;
  Result FillFunctionTable(SwapChainInterface &table) const override;
  Result FillFunctionTable(UpscalerInterface &table) const override;

#if NRI_ENABLE_IMGUI_EXTENSION
  Result FillFunctionTable(ImguiInterface &table) const override;
#endif

  private:
    DeviceInfo m_Desc = {};
};

Result CreateDeviceNONE(const DeviceCreationDesc &desc, DeviceBase *&device) {
  DeviceNONE *impl = Allocate<DeviceNONE>(desc.allocationCallbacks, desc.callbackInterface, desc.allocationCallbacks,
                                          desc.adapterDesc);

  if (!impl) {
    Destroy(desc.allocationCallbacks, impl);
    device = nullptr;

    return Result::Failure;
  }

  device = (DeviceBase*)impl;

  return Result::Success;
}

//============================================================================================================================================================================================
#pragma region[  Core  ]

static const DeviceInfo &GetDeviceDesc(const Device &device) {
  return ((DeviceNONE&)device).GetDesc();
}

static const BufferDesc &GetBufferDesc(const Buffer &) {
  static const BufferDesc bufferDesc = {1};

  return bufferDesc;
}

static const TextureDesc &GetTextureDesc(const Texture &) {
  static const TextureDesc textureDesc = {
    TextureDimension::Texture1D, TextureUsageBits::None, Format::R8_UNORM, 1, 1, 1, 1, 1, 1
  };

  return textureDesc;
}

static FormatSupportBits GetFormatSupport(const Device &, Format) {
  return (FormatSupportBits)(-1);
}

static Result GetQueue(Device &, QueueType, uint32_t, Queue *&queue) {
  queue = DummyObject<Queue>();

  return Result::Success;
}

static Result CreateCommandAllocator(Queue &, CommandAllocator *&commandAllocator) {
  commandAllocator = DummyObject<CommandAllocator>();

  return Result::Success;
}

static Result CreateCommandBuffer(CommandAllocator &, CommandBuffer *&commandBuffer) {
  commandBuffer = DummyObject<CommandBuffer>();

  return Result::Success;
}

static Result CreateFence(Device &, uint64_t, Fence *&fence) {
  fence = DummyObject<Fence>();

  return Result::Success;
}

static Result CreateDescriptorPool(Device &, const DescriptorPoolDesc &, DescriptorPool *&descriptorPool) {
  descriptorPool = DummyObject<DescriptorPool>();

  return Result::Success;
}

static Result CreatePipelineLayout(Device &, const PipelineLayoutDesc &, PipelineLayout *&pipelineLayout) {
  pipelineLayout = DummyObject<PipelineLayout>();

  return Result::Success;
}

static Result CreateGraphicsPipeline(Device &, const GraphicsPipelineDesc &, Pipeline *&pipeline) {
  pipeline = DummyObject<Pipeline>();

  return Result::Success;
}

static Result CreateComputePipeline(Device &, const ComputePipelineDesc &, Pipeline *&pipeline) {
  pipeline = DummyObject<Pipeline>();

  return Result::Success;
}

static Result CreatePipelineCache(Device &, const PipelineCacheDesc &, PipelineCache *&pipelineCache) {
  pipelineCache = DummyObject<PipelineCache>();

  return Result::Success;
}

static Result CreateQueryPool(Device &, const QueryPoolDesc &, QueryPool *&queryPool) {
  queryPool = DummyObject<QueryPool>();

  return Result::Success;
}

static Result CreateSampler(Device &, const SamplerDesc &, Descriptor *&sampler) {
  sampler = DummyObject<Descriptor>();

  return Result::Success;
}

static Result CreateBufferView(const BufferViewDesc &, Descriptor *&bufferView) {
  bufferView = DummyObject<Descriptor>();

  return Result::Success;
}

static Result CreateTextureView(const TextureViewDesc &, Descriptor *&textureView) {
  textureView = DummyObject<Descriptor>();

  return Result::Success;
}

static void DestroyCommandAllocator(CommandAllocator *) {
}

static void DestroyCommandBuffer(CommandBuffer *) {
}

static void DestroyDescriptorPool(DescriptorPool *) {
}

static void DestroyBuffer(Buffer *) {
}

static void DestroyTexture(Texture *) {
}

static void DestroyDescriptor(Descriptor *) {
}

static void DestroyPipelineLayout(PipelineLayout *) {
}

static void DestroyPipeline(Pipeline *) {
}

static void DestroyPipelineCache(PipelineCache *) {
}

static Result GetPipelineCacheData(PipelineCache &, void *, uint64_t &size) {
  size = 0;
  return Result::Success;
}

static void DestroyQueryPool(QueryPool *) {
}

static void DestroyFence(Fence *) {
}

static Result AllocateMemory(Device &, const AllocateMemoryDesc &, Memory *&memory) {
  memory = DummyObject<Memory>();

  return Result::Success;
}

static void FreeMemory(Memory *) {
}

static Result CreateBuffer(Device &, const BufferDesc &, Buffer *&buffer) {
  buffer = DummyObject<Buffer>();

  return Result::Success;
}

static Result CreateTexture(Device &, const TextureDesc &, Texture *&texture) {
  texture = DummyObject<Texture>();

  return Result::Success;
}

static void GetBufferMemoryDesc(const Buffer &, MemoryLocation, MemoryDesc &memoryDesc) {
  memoryDesc = {1};
}

static void GetTextureMemoryDesc(const Texture &, MemoryLocation, MemoryDesc &memoryDesc) {
  memoryDesc = {1};
}

static Result BindBufferMemory(const BindBufferMemoryDesc *, uint32_t) {
  return Result::Success;
}

static Result BindTextureMemory(const BindTextureMemoryDesc *, uint32_t) {
  return Result::Success;
}

static void GetBufferMemoryDesc2(const Device &, const BufferDesc &, MemoryLocation, MemoryDesc &memoryDesc) {
  memoryDesc = {1};
}

static void GetTextureMemoryDesc2(const Device &, const TextureDesc &, MemoryLocation, MemoryDesc &memoryDesc) {
  memoryDesc = {1};
}

static Result CreateCommittedBuffer(Device &, MemoryLocation, float, const BufferDesc &, Buffer *&buffer) {
  buffer = DummyObject<Buffer>();

  return Result::Success;
}

static Result CreateCommittedTexture(Device &, MemoryLocation, float, const TextureDesc &, Texture *&texture) {
  texture = DummyObject<Texture>();

  return Result::Success;
}

static Result CreatePlacedBuffer(Device &, Memory *, uint64_t, const BufferDesc &, Buffer *&buffer) {
  buffer = DummyObject<Buffer>();

  return Result::Success;
}

static Result CreatePlacedTexture(Device &, Memory *, uint64_t, const TextureDesc &, Texture *&texture) {
  texture = DummyObject<Texture>();

  return Result::Success;
}

static Result AllocateDescriptorSets(DescriptorPool &, const PipelineLayout &, uint32_t, DescriptorSet **, uint32_t,
                                     uint32_t) {
  return Result::Success;
}

static void UpdateDescriptorRanges(const UpdateDescriptorRangeDesc *, uint32_t) {
}

static void CopyDescriptorRanges(const CopyDescriptorRangeDesc *, uint32_t) {
}

static void ResetDescriptorPool(DescriptorPool &) {
}

static void GetDescriptorSetOffsets(const DescriptorSet &, uint32_t &resourceHeapOffset, uint32_t &samplerHeapOffset) {
  resourceHeapOffset = 0;
  samplerHeapOffset = 0;
}

static Result BeginCommandBuffer(CommandBuffer &, const DescriptorPool *) {
  return Result::Success;
}

static void CmdSetDescriptorPool(CommandBuffer &, const DescriptorPool &) {
}

static void CmdSetPipelineLayout(CommandBuffer &, BindPoint, const PipelineLayout &) {
}

static void CmdSetDescriptorSet(CommandBuffer &, const SetDescriptorSetDesc &) {
}

static void CmdSetRootConstants(CommandBuffer &, const SetRootConstantsDesc &) {
}

static void CmdSetRootDescriptor(CommandBuffer &, const SetRootDescriptorDesc &) {
}

static void CmdSetPipeline(CommandBuffer &, const Pipeline &) {
}

static void CmdBarrier(CommandBuffer &, const BarrierDesc &) {
}

static void CmdSetIndexBuffer(CommandBuffer &, const Buffer &, uint64_t, IndexType) {
}

static void CmdSetVertexBuffers(CommandBuffer &, uint32_t, const VertexBufferDesc *, uint32_t) {
}

static void CmdSetViewports(CommandBuffer &, const Viewport *, uint32_t) {
}

static void CmdSetScissors(CommandBuffer &, const Rect *, uint32_t) {
}

static void CmdSetStencilReference(CommandBuffer &, uint8_t, uint8_t) {
}

static void CmdSetDepthBounds(CommandBuffer &, float, float) {
}

static void CmdSetBlendConstants(CommandBuffer &, const Color32f &) {
}

static void CmdSetSampleLocations(CommandBuffer &, const SampleLocation *, Sample_t, Sample_t) {
}

static void CmdSetShadingRate(CommandBuffer &, const ShadingRateDesc &) {
}

static void CmdSetDepthBias(CommandBuffer &, const DepthBiasDesc &) {
}

static void CmdBeginRendering(CommandBuffer &, const RenderingDesc &) {
}

static void CmdClearAttachments(CommandBuffer &, const ClearAttachmentDesc *, uint32_t, const Rect *, uint32_t) {
}

static void CmdDraw(CommandBuffer &, const DrawDesc &) {
}

static void CmdDrawIndexed(CommandBuffer &, const DrawIndexedDesc &) {
}

static void CmdDrawIndirect(CommandBuffer &, const Buffer &, uint64_t, uint32_t, uint32_t, const Buffer *, uint64_t) {
}

static void CmdDrawIndexedIndirect(CommandBuffer &, const Buffer &, uint64_t, uint32_t, uint32_t, const Buffer *,
                                   uint64_t) {
}

static void CmdEndRendering(CommandBuffer &) {
}

static void CmdDispatch(CommandBuffer &, const DispatchDesc &) {
}

static void CmdDispatchIndirect(CommandBuffer &, const Buffer &, uint64_t) {
}

static void CmdCopyBuffer(CommandBuffer &, Buffer &, uint64_t, const Buffer &, uint64_t, uint64_t) {
}

static void CmdCopyTexture(CommandBuffer &, Texture &, const TextureRegionDesc *, const Texture &,
                           const TextureRegionDesc *) {
}

static void CmdUploadBufferToTexture(CommandBuffer &, Texture &, const TextureRegionDesc &, const Buffer &,
                                     const TextureDataLayoutDesc &) {
}

static void CmdReadbackTextureToBuffer(CommandBuffer &, Buffer &, const TextureDataLayoutDesc &, const Texture &,
                                       const TextureRegionDesc &) {
}

static void CmdZeroBuffer(CommandBuffer &, Buffer &, uint64_t, uint64_t) {
}

static void CmdResolveTexture(CommandBuffer &, Texture &, const TextureRegionDesc *, const Texture &,
                              const TextureRegionDesc *, ResolveOp) {
}

static void CmdClearStorage(CommandBuffer &, const ClearStorageDesc &) {
}

static void CmdResetQueries(CommandBuffer &, QueryPool &, uint32_t, uint32_t) {
}

static void CmdBeginQuery(CommandBuffer &, QueryPool &, uint32_t) {
}

static void CmdEndQuery(CommandBuffer &, QueryPool &, uint32_t) {
}

static void CmdCopyQueries(CommandBuffer &, const QueryPool &, uint32_t, uint32_t, Buffer &, uint64_t) {
}

static void CmdBeginAnnotation(CommandBuffer &, const char *, uint32_t) {
}

static void CmdEndAnnotation(CommandBuffer &) {
}

static void CmdAnnotation(CommandBuffer &, const char *, uint32_t) {
}

static Result EndCommandBuffer(CommandBuffer &) {
  return Result::Success;
}

static void QueueBeginAnnotation(Queue &, const char *, uint32_t) {
}

static void QueueEndAnnotation(Queue &) {
}

static void QueueAnnotation(Queue &, const char *, uint32_t) {
}

static void GetCalibratedTimestamps(Queue &, uint64_t &timestampGPU, uint64_t &timestampCPU) {
  timestampGPU = 0;
  timestampCPU = 0;
}

static void ResetQueries(QueryPool &, uint32_t, uint32_t) {
}

static uint32_t GetQuerySize(const QueryPool &) {
  return 0;
}

static Result QueueSubmit(Queue &, const QueueSubmitDesc &) {
  return Result::Success;
}

static Result DeviceWaitIdle(Device *) {
  return Result::Success;
}

static Result QueueWaitIdle(Queue *) {
  return Result::Success;
}

static void Wait(Fence &, uint64_t) {
}

static uint64_t GetFenceValue(Fence &) {
  return 0;
}

static void ResetCommandAllocator(CommandAllocator &) {
}

static void *MapBuffer(Buffer &, uint64_t, uint64_t) {
  return nullptr;
}

static void UnmapBuffer(Buffer &) {
}

static uint64_t GetBufferDeviceAddress(const Buffer &) {
  return 0;
}

static void SetDebugName(Object *, const char *) {
}

static void *GetDeviceNativeObject(const Device *) {
  return nullptr;
}

static void *GetQueueNativeObject(const Queue *) {
  return nullptr;
}

static void *GetCommandBufferNativeObject(const CommandBuffer *) {
  return nullptr;
}

static uint64_t GetBufferNativeObject(const Buffer *) {
  return 0;
}

static uint64_t GetTextureNativeObject(const Texture *) {
  return 0;
}

static uint64_t GetDescriptorNativeObject(const Descriptor *) {
  return 0;
}

Result DeviceNONE::FillFunctionTable(CoreInterface &table) const {
  table.getDeviceDesc = ::GetDeviceDesc;
  table.getBufferDesc = ::GetBufferDesc;
  table.getTextureDesc = ::GetTextureDesc;
  table.getFormatSupport = ::GetFormatSupport;
  table.getQuerySize = ::GetQuerySize;
  table.getFenceValue = ::GetFenceValue;
  table.getDescriptorSetOffsets = ::GetDescriptorSetOffsets;
  table.getQueue = ::GetQueue;
  table.createCommandAllocator = ::CreateCommandAllocator;
  table.createCommandBuffer = ::CreateCommandBuffer;
  table.createDescriptorPool = ::CreateDescriptorPool;
  table.createBufferView = ::CreateBufferView;
  table.createTextureView = ::CreateTextureView;
  table.createSampler = ::CreateSampler;
  table.createPipelineLayout = ::CreatePipelineLayout;
  table.createGraphicsPipeline = ::CreateGraphicsPipeline;
  table.createComputePipeline = ::CreateComputePipeline;
  table.createPipelineCache = ::CreatePipelineCache;
  table.createQueryPool = ::CreateQueryPool;
  table.createFence = ::CreateFence;
  table.destroyCommandAllocator = ::DestroyCommandAllocator;
  table.destroyCommandBuffer = ::DestroyCommandBuffer;
  table.destroyDescriptorPool = ::DestroyDescriptorPool;
  table.destroyBuffer = ::DestroyBuffer;
  table.destroyTexture = ::DestroyTexture;
  table.destroyDescriptor = ::DestroyDescriptor;
  table.destroyPipelineLayout = ::DestroyPipelineLayout;
  table.destroyPipeline = ::DestroyPipeline;
  table.destroyPipelineCache = ::DestroyPipelineCache;
  table.getPipelineCacheData = ::GetPipelineCacheData;
  table.destroyQueryPool = ::DestroyQueryPool;
  table.destroyFence = ::DestroyFence;
  table.allocateMemory = ::AllocateMemory;
  table.freeMemory = ::FreeMemory;
  table.createBuffer = ::CreateBuffer;
  table.createTexture = ::CreateTexture;
  table.getBufferMemoryDesc = ::GetBufferMemoryDesc;
  table.getTextureMemoryDesc = ::GetTextureMemoryDesc;
  table.bindBufferMemory = ::BindBufferMemory;
  table.bindTextureMemory = ::BindTextureMemory;
  table.getBufferMemoryDesc2 = ::GetBufferMemoryDesc2;
  table.getTextureMemoryDesc2 = ::GetTextureMemoryDesc2;
  table.createCommittedBuffer = ::CreateCommittedBuffer;
  table.createCommittedTexture = ::CreateCommittedTexture;
  table.createPlacedBuffer = ::CreatePlacedBuffer;
  table.createPlacedTexture = ::CreatePlacedTexture;
  table.allocateDescriptorSets = ::AllocateDescriptorSets;
  table.updateDescriptorRanges = ::UpdateDescriptorRanges;
  table.copyDescriptorRanges = ::CopyDescriptorRanges;
  table.resetDescriptorPool = ::ResetDescriptorPool;
  table.beginCommandBuffer = ::BeginCommandBuffer;
  table.cmdSetDescriptorPool = ::CmdSetDescriptorPool;
  table.cmdSetDescriptorSet = ::CmdSetDescriptorSet;
  table.cmdSetPipelineLayout = ::CmdSetPipelineLayout;
  table.cmdSetPipeline = ::CmdSetPipeline;
  table.cmdSetRootConstants = ::CmdSetRootConstants;
  table.cmdSetRootDescriptor = ::CmdSetRootDescriptor;
  table.cmdBarrier = ::CmdBarrier;
  table.cmdSetIndexBuffer = ::CmdSetIndexBuffer;
  table.cmdSetVertexBuffers = ::CmdSetVertexBuffers;
  table.cmdSetViewports = ::CmdSetViewports;
  table.cmdSetScissors = ::CmdSetScissors;
  table.cmdSetStencilReference = ::CmdSetStencilReference;
  table.cmdSetDepthBounds = ::CmdSetDepthBounds;
  table.cmdSetBlendConstants = ::CmdSetBlendConstants;
  table.cmdSetSampleLocations = ::CmdSetSampleLocations;
  table.cmdSetShadingRate = ::CmdSetShadingRate;
  table.cmdSetDepthBias = ::CmdSetDepthBias;
  table.cmdBeginRendering = ::CmdBeginRendering;
  table.cmdClearAttachments = ::CmdClearAttachments;
  table.cmdDraw = ::CmdDraw;
  table.cmdDrawIndexed = ::CmdDrawIndexed;
  table.cmdDrawIndirect = ::CmdDrawIndirect;
  table.cmdDrawIndexedIndirect = ::CmdDrawIndexedIndirect;
  table.cmdEndRendering = ::CmdEndRendering;
  table.cmdDispatch = ::CmdDispatch;
  table.cmdDispatchIndirect = ::CmdDispatchIndirect;
  table.cmdCopyBuffer = ::CmdCopyBuffer;
  table.cmdCopyTexture = ::CmdCopyTexture;
  table.cmdUploadBufferToTexture = ::CmdUploadBufferToTexture;
  table.cmdReadbackTextureToBuffer = ::CmdReadbackTextureToBuffer;
  table.cmdZeroBuffer = ::CmdZeroBuffer;
  table.cmdResolveTexture = ::CmdResolveTexture;
  table.cmdClearStorage = ::CmdClearStorage;
  table.cmdResetQueries = ::CmdResetQueries;
  table.cmdBeginQuery = ::CmdBeginQuery;
  table.cmdEndQuery = ::CmdEndQuery;
  table.cmdCopyQueries = ::CmdCopyQueries;
  table.cmdBeginAnnotation = ::CmdBeginAnnotation;
  table.cmdEndAnnotation = ::CmdEndAnnotation;
  table.cmdAnnotation = ::CmdAnnotation;
  table.endCommandBuffer = ::EndCommandBuffer;
  table.queueBeginAnnotation = ::QueueBeginAnnotation;
  table.queueEndAnnotation = ::QueueEndAnnotation;
  table.queueAnnotation = ::QueueAnnotation;
  table.getCalibratedTimestamps = ::GetCalibratedTimestamps;
  table.resetQueries = ::ResetQueries;
  table.queueSubmit = ::QueueSubmit;
  table.queueWaitIdle = ::QueueWaitIdle;
  table.deviceWaitIdle = ::DeviceWaitIdle;
  table.wait = ::Wait;
  table.resetCommandAllocator = ::ResetCommandAllocator;
  table.mapBuffer = ::MapBuffer;
  table.unmapBuffer = ::UnmapBuffer;
  table.getBufferDeviceAddress = ::GetBufferDeviceAddress;
  table.setDebugName = ::SetDebugName;
  table.getDeviceNativeObject = ::GetDeviceNativeObject;
  table.getQueueNativeObject = ::GetQueueNativeObject;
  table.getCommandBufferNativeObject = ::GetCommandBufferNativeObject;
  table.getBufferNativeObject = ::GetBufferNativeObject;
  table.getTextureNativeObject = ::GetTextureNativeObject;
  table.getDescriptorNativeObject = ::GetDescriptorNativeObject;

  return Result::Success;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  Helper  ]

static uint32_t CalculateAllocationNumber(const Device &, const ResourceGroupDesc &) {
  return 0;
}

static Result AllocateAndBindMemory(Device &, const ResourceGroupDesc &, Memory **) {
  return Result::Success;
}

static Result UploadData(Queue &, const TextureUploadDesc *, uint32_t, const BufferUploadDesc *, uint32_t) {
  return Result::Success;
}

static Result QueryVideoMemoryInfo(const Device &, MemoryLocation, VideoMemoryInfo &videoMemoryInfo) {
  videoMemoryInfo = {};

  return Result::Success;
}

Result DeviceNONE::FillFunctionTable(HelperInterface &table) const {
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

static Result CreateImgui(Device &, const ImguiDesc &, Imgui *&imgui) {
  imgui = DummyObject<Imgui>();

  return Result::Success;
}

static void DestroyImgui(Imgui *) {
}

static void CmdCopyImguiData(CommandBuffer &, Streamer &, Imgui &, const CopyImguiDataDesc &) {
}

static void CmdDrawImgui(CommandBuffer &, Imgui &, const DrawImguiDesc &) {
}

Result DeviceNONE::FillFunctionTable(ImguiInterface &table) const {
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

static Result SetLatencySleepMode(SwapChain &, const LatencySleepMode &) {
  return Result::Success;
}

static Result SetLatencyMarker(SwapChain &, LatencyMarker) {
  return Result::Success;
}

static Result LatencySleep(SwapChain &) {
  return Result::Success;
}

static Result GetLatencyReport(const SwapChain &, LatencyReport &) {
  return Result::Success;
}

Result DeviceNONE::FillFunctionTable(LowLatencyInterface &table) const {
  table.SetLatencySleepMode = ::SetLatencySleepMode;
  table.SetLatencyMarker = ::SetLatencyMarker;
  table.LatencySleep = ::LatencySleep;
  table.GetLatencyReport = ::GetLatencyReport;

  return Result::Success;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  MeshShader  ]

static void CmdDrawMeshTasks(CommandBuffer &, const DrawMeshTasksDesc &) {
}

static void CmdDrawMeshTasksIndirect(CommandBuffer &, const Buffer &, uint64_t, uint32_t, uint32_t, const Buffer *,
                                     uint64_t) {
}

Result DeviceNONE::FillFunctionTable(MeshShaderInterface &table) const {
  table.CmdDrawMeshTasks = ::CmdDrawMeshTasks;
  table.CmdDrawMeshTasksIndirect = ::CmdDrawMeshTasksIndirect;

  return Result::Success;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  RayTracing  ]

static Result CreateRayTracingPipeline(Device &, const RayTracingPipelineDesc &, Pipeline *&pipeline) {
  pipeline = DummyObject<Pipeline>();

  return Result::Success;
}

static Result CreateAccelerationStructureDescriptor(const AccelerationStructure &, Descriptor *&descriptor) {
  descriptor = DummyObject<Descriptor>();

  return Result::Success;
}

static uint64_t GetAccelerationStructureHandle(const AccelerationStructure &) {
  return 0;
}

static uint64_t GetAccelerationStructureUpdateScratchBufferSize(const AccelerationStructure &) {
  return 0;
}

static uint64_t GetAccelerationStructureBuildScratchBufferSize(const AccelerationStructure &) {
  return 0;
}

static uint64_t GetMicromapBuildScratchBufferSize(const Micromap &) {
  return 0;
}

static Buffer *GetAccelerationStructureBuffer(const AccelerationStructure &) {
  return DummyObject<Buffer>();
}

static Buffer *GetMicromapBuffer(const Micromap &) {
  return DummyObject<Buffer>();
}

static void DestroyAccelerationStructure(AccelerationStructure *) {
}

static void DestroyMicromap(Micromap *) {
}

static Result CreateAccelerationStructure(Device &, const AccelerationStructureDesc &,
                                          AccelerationStructure *&accelerationStructure) {
  accelerationStructure = DummyObject<AccelerationStructure>();

  return Result::Success;
}

static Result CreateMicromap(Device &, const MicromapDesc &, Micromap *&micromap) {
  micromap = DummyObject<Micromap>();

  return Result::Success;
}

static void GetAccelerationStructureMemoryDesc(const AccelerationStructure &, MemoryLocation, MemoryDesc &memoryDesc) {
  memoryDesc = {1};
}

static void GetMicromapMemoryDesc(const Micromap &, MemoryLocation, MemoryDesc &memoryDesc) {
  memoryDesc = {1};
}

static Result BindAccelerationStructureMemory(const BindAccelerationStructureMemoryDesc *, uint32_t) {
  return Result::Success;
}

static Result BindMicromapMemory(const BindMicromapMemoryDesc *, uint32_t) {
  return Result::Success;
}

static void GetAccelerationStructureMemoryDesc2(const Device &, const AccelerationStructureDesc &, MemoryLocation,
                                                MemoryDesc &memoryDesc) {
  memoryDesc = {1};
}

static void GetMicromapMemoryDesc2(const Device &, const MicromapDesc &, MemoryLocation, MemoryDesc &memoryDesc) {
  memoryDesc = {1};
}

static Result CreateCommittedAccelerationStructure(Device &, MemoryLocation, float, const AccelerationStructureDesc &,
                                                   AccelerationStructure *&accelerationStructure) {
  accelerationStructure = DummyObject<AccelerationStructure>();

  return Result::Success;
}

static Result CreateCommittedMicromap(Device &, MemoryLocation, float, const MicromapDesc &, Micromap *&micromap) {
  micromap = DummyObject<Micromap>();

  return Result::Success;
}

static Result CreatePlacedAccelerationStructure(Device &, Memory *, uint64_t, const AccelerationStructureDesc &,
                                                AccelerationStructure *&accelerationStructure) {
  accelerationStructure = DummyObject<AccelerationStructure>();

  return Result::Success;
}

static Result CreatePlacedMicromap(Device &, Memory *, uint64_t, const MicromapDesc &, Micromap *&micromap) {
  micromap = DummyObject<Micromap>();

  return Result::Success;
}

static Result WriteShaderGroupIdentifiers(const Pipeline &, uint32_t, uint32_t, void *) {
  return Result::Success;
}

static void CmdBuildTopLevelAccelerationStructures(CommandBuffer &, const BuildTopLevelAccelerationStructureDesc *,
                                                   uint32_t) {
}

static void CmdBuildBottomLevelAccelerationStructures(CommandBuffer &,
                                                      const BuildBottomLevelAccelerationStructureDesc *, uint32_t) {
}

static void CmdBuildMicromaps(CommandBuffer &, const BuildMicromapDesc *, uint32_t) {
}

static void CmdDispatchRays(CommandBuffer &, const DispatchRaysDesc &) {
}

static void CmdDispatchRaysIndirect(CommandBuffer &, const Buffer &, uint64_t) {
}

static void CmdWriteAccelerationStructuresSizes(CommandBuffer &, const AccelerationStructure *const*, uint32_t,
                                                QueryPool &, uint32_t) {
}

static void CmdWriteMicromapsSizes(CommandBuffer &, const Micromap *const*, uint32_t, QueryPool &, uint32_t) {
}

static void CmdCopyAccelerationStructure(CommandBuffer &, AccelerationStructure &, const AccelerationStructure &,
                                         CopyMode) {
}

static void CmdCopyMicromap(CommandBuffer &, Micromap &, const Micromap &, CopyMode) {
}

static uint64_t GetAccelerationStructureNativeObject(const AccelerationStructure *) {
  return 0;
}

static uint64_t GetMicromapNativeObject(const Micromap *) {
  return 0;
}

Result DeviceNONE::FillFunctionTable(RayTracingInterface &table) const {
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

static Result CreateStreamer(Device &, const StreamerDesc &, Streamer *&streamer) {
  streamer = DummyObject<Streamer>();

  return Result::Success;
}

static void DestroyStreamer(Streamer *) {
}

static Buffer *GetStreamerConstantBuffer(Streamer &) {
  return nullptr;
}

static uint32_t StreamConstantData(Streamer &, const void *, uint32_t) {
  return 0;
}

static BufferOffset StreamBufferData(Streamer &, const StreamBufferDataDesc &) {
  return {};
}

static BufferOffset StreamTextureData(Streamer &, const StreamTextureDataDesc &) {
  return {};
}

static void EndStreamerFrame(Streamer &) {
}

static void CmdCopyStreamedData(CommandBuffer &, Streamer &) {
}

Result DeviceNONE::FillFunctionTable(StreamerInterface &table) const {
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

static Result CreateSwapChain(Device &, const SwapChainDesc &, SwapChain *&swapChain) {
  swapChain = DummyObject<SwapChain>();

  return Result::Success;
}

static void DestroySwapChain(SwapChain *) {
}

static Texture *const*GetSwapChainTextures(const SwapChain &, uint32_t &textureNum) {
  static const void *textures[1] = {};
  textureNum = 1;

  return (Texture**)textures;
}

static Result GetDisplayDesc(SwapChain &, DisplayDesc &displayDesc) {
  displayDesc = {};

  return Result::Success;
}

static Result AcquireNextTexture(SwapChain &, Fence &, uint32_t &textureIndex) {
  textureIndex = 0;

  return Result::Success;
}

static Result WaitForPresent(SwapChain &) {
  return Result::Success;
}

static Result QueuePresent(SwapChain &, Fence &) {
  return Result::Success;
}

Result DeviceNONE::FillFunctionTable(SwapChainInterface &table) const {
  table.createSwapChain = ::CreateSwapChain;
  table.destroySwapChain = ::DestroySwapChain;
  table.getSwapChainTextures = ::GetSwapChainTextures;
  table.getDisplayDesc = ::GetDisplayDesc;
  table.acquireNextTexture = ::AcquireNextTexture;
  table.waitForPresent = ::WaitForPresent;
  table.queuePresent = ::QueuePresent;

  return Result::Success;
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region[  Upscaler  ]

static Result CreateUpscaler(Device &, const UpscalerDesc &, Upscaler *&upscaler) {
  upscaler = DummyObject<Upscaler>();

  return Result::Success;
}

static void DestroyUpscaler(Upscaler *) {
}

static bool IsUpscalerSupported(const Device &, UpscalerType) {
  return true;
}

static void GetUpscalerProps(const Upscaler &, UpscalerProps &upscalerProps) {
  upscalerProps = {1.0f, 0.0f, {1, 1}, {1, 1}, {1, 1}, 1};
}

static void CmdDispatchUpscale(CommandBuffer &, Upscaler &, const DispatchUpscaleDesc &) {
}

Result DeviceNONE::FillFunctionTable(UpscalerInterface &table) const {
  table.CreateUpscaler = ::CreateUpscaler;
  table.DestroyUpscaler = ::DestroyUpscaler;
  table.IsUpscalerSupported = ::IsUpscalerSupported;
  table.GetUpscalerProps = ::GetUpscalerProps;
  table.CmdDispatchUpscale = ::CmdDispatchUpscale;

  return Result::Success;
}

#pragma endregion
