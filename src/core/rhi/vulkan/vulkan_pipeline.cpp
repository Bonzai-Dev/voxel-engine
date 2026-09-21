#include "vulkan_backend.hpp"
#include "vulkan_device.hpp"
#include "vulkan_pipeline_cache.hpp"
#include "vulkan_pipeline_layout.hpp"

namespace {
  using namespace Core::RHI;

  inline bool constantColorReferenced(BlendFactor factor) {
    return factor == BlendFactor::ConstantColor ||
      factor == BlendFactor::ConstantAlpha ||
      factor == BlendFactor::OneMinusConstantColor ||
      factor == BlendFactor::OneMinusConstantAlpha;
  }

  void addInputAttachmentIndicesFromSpirv(std::vector<uint32_t> &inputAttachmentIndices, const ShaderInfo &shaderDesc) {
    constexpr uint32_t SPIRV_MAGIC = 0x07230203;
    constexpr uint16_t SPIRV_OP_DECORATE = 71;
    constexpr uint32_t SPIRV_DECORATION_INPUT_ATTACHMENT_INDEX = 43;

    // Remove this parser only if "GraphicsPipelineDesc" provides input attachment indices or NRI requires "attachmentIndex == bindingIndex"
    if (!(shaderDesc.stage & StageBits::FragmentShader) || !shaderDesc.bytecode || shaderDesc.size < 5 * sizeof(
      uint32_t) || (shaderDesc.size & 3))
      return;

    const uint32_t *code = (const uint32_t*)shaderDesc.bytecode;
    uint32_t wordNum = (uint32_t)(shaderDesc.size / sizeof(uint32_t));
    if (code[0] != SPIRV_MAGIC)
      return;

    for (uint32_t offset = 5; offset < wordNum;) {
      uint32_t instruction = code[offset];
      uint16_t wordCount = (uint16_t)(instruction >> 16);
      uint16_t opCode = (uint16_t)instruction;

      if (!wordCount || offset + wordCount > wordNum)
        break;

      if (opCode == SPIRV_OP_DECORATE && wordCount >= 4 && code[offset + 2] == SPIRV_DECORATION_INPUT_ATTACHMENT_INDEX)
        setRenderPassInputAttachmentIndex(inputAttachmentIndices, code[offset + 3]);

      offset += wordCount;
    }
  }

  void fillRenderPassInputAttachmentIndices(
    std::vector<uint32_t> &inputAttachmentIndices, const GraphicsPipelineInfo &graphicsPipelineDesc
  ) {
    for (uint32_t i = 0; i < graphicsPipelineDesc.shaderCount; i++)
      addInputAttachmentIndicesFromSpirv(inputAttachmentIndices, graphicsPipelineDesc.shaders[i]);
  }

  bool fillPipelineRobustness(
    const VulkanDevice &device, Robustness robustness, VkPipelineRobustnessCreateInfoEXT &robustnessInfo
  ) {
    if (!device.getVulkanFeatures().pipelineRobustness || robustness == Robustness::Default)
      return false;

    if (!device.getVulkanFeatures().robustness2)
      robustness = robustness == Robustness::D3D12 ? Robustness::Vulkan : robustness;
    if (!device.getVulkanFeatures().robustness)
      robustness = robustness == Robustness::Vulkan ? Robustness::Off : robustness;

    if (robustness == Robustness::Vulkan) {
      robustnessInfo.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT;
      robustnessInfo.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
      robustnessInfo.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
      robustnessInfo.vertexInputs = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
    }
    else if (robustness == Robustness::D3D12) {
      robustnessInfo.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2_EXT;
      robustnessInfo.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
      robustnessInfo.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
      robustnessInfo.vertexInputs = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
    }
    else if (robustness == Robustness::Off) {
      robustnessInfo.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DISABLED_EXT;
      robustnessInfo.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT;
      robustnessInfo.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT;
      robustnessInfo.vertexInputs = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT;
    }

    return true;
  }

  void fillRenderPassInputAttachmentCompatibilityIndices(
    std::vector<uint32_t> &inputAttachmentIndices, const GraphicsPipelineInfo &graphicsPipelineDesc
  ) {
    fillRenderPassInputAttachmentIndices(inputAttachmentIndices, graphicsPipelineDesc);
    if (!inputAttachmentIndices.empty())
      return;

    const VulkanPipelineLayout &pipelineLayoutVK = *(VulkanPipelineLayout*)graphicsPipelineDesc.pipelineLayout;
    const BindingInfo &bindingInfo = pipelineLayoutVK.getBindingInfo();

    for (const DescriptorRangeDesc &range : bindingInfo.ranges) {
      if (range.descriptorType != DescriptorType::InputAttachment)
        continue;

      for (uint32_t i = 0; i < range.descriptorNum; i++) {
        uint32_t index = range.baseRegisterIndex + i;
        if (index < graphicsPipelineDesc.outputMerger.colorCount)
          setRenderPassInputAttachmentIndex(inputAttachmentIndices, index);
      }
    }
  }
}

namespace Core::RHI {
  Result VulkanPipeline::create(const GraphicsPipelineInfo &graphicsPipelineDesc) {
    bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Shaders
    std::vector<VkPipelineShaderStageCreateInfo> stages(graphicsPipelineDesc.shaderCount);
    std::vector<VkShaderModule> modules(graphicsPipelineDesc.shaderCount);

    for (uint32_t i = 0; i < graphicsPipelineDesc.shaderCount; i++) {
      const ShaderInfo &shaderDesc = graphicsPipelineDesc.shaders[i];
      Result res = setupShaderStage(stages[i], shaderDesc, modules[i]);
      if (res != Result::Success)
        return res;

      stages[i].pName = shaderDesc.entryPointName ? shaderDesc.entryPointName : "main";
    }

    // Vertex input
    const VertexInputInfo *vi = graphicsPipelineDesc.vertexInput;
    uint32_t attributeCount = vi ? vi->attributeCount : 0u;
    uint32_t streamCount = vi ? vi->streamCount : 0u;

    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescs(attributeCount);
    std::vector<VkVertexInputBindingDescription> vertexBindingDescs(streamCount);

    VkPipelineVertexInputStateCreateInfo vertexInputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescs.data();
    vertexInputState.pVertexBindingDescriptions = vertexBindingDescs.data();

    if (vi) {
      vertexInputState.vertexAttributeDescriptionCount = vi->attributeCount;
      vertexInputState.vertexBindingDescriptionCount = vi->streamCount;

      for (uint32_t i = 0; i < vi->attributeCount; i++) {
        const VertexAttributeInfo &attribute = vi->attributes[i];

        VkVertexInputAttributeDescription &vertexAttributeDesc = vertexAttributeDescs[i];
        vertexAttributeDesc = {};
        vertexAttributeDesc.location = attribute.vulkan.location;
        vertexAttributeDesc.binding = attribute.streamIndex;
        vertexAttributeDesc.format = getVulkanFormat(attribute.format);
        vertexAttributeDesc.offset = attribute.offset;
      }

      for (uint32_t i = 0; i < vi->streamCount; i++) {
        const VertexStreamInfo &stream = vi->streams[i];

        VkVertexInputBindingDescription &vertexBindingDesc = vertexBindingDescs[i];
        vertexBindingDesc = {};
        vertexBindingDesc.binding = stream.bindingSlot;
        vertexBindingDesc.inputRate = stream.stepRate == VertexStreamStepRate::PerVertex
                                        ? VK_VERTEX_INPUT_RATE_VERTEX
                                        : VK_VERTEX_INPUT_RATE_INSTANCE;
        if (!device.getInfo().features.extendedDynamicState)
          vertexBindingDesc.stride = stream.stride;
      }
    }

    // Input assembly
    const InputAssemblyInfo &ia = graphicsPipelineDesc.inputAssembly;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
    };
    inputAssemblyState.topology = getVulkanTopology(ia.topology);
    inputAssemblyState.primitiveRestartEnable = ia.primitiveRestart != PrimitiveRestart::Disabled;

    VkPipelineTessellationStateCreateInfo tessellationState = {
      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO
    };
    tessellationState.patchControlPoints = ia.tessControlPointNum;

    // Multisample
    const MultisampleInfo *ms = graphicsPipelineDesc.multisample;

    VkPipelineSampleLocationsStateCreateInfoEXT sampleLocationsState = {
      VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT
    };
    sampleLocationsState.sampleLocationsInfo.sType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;

    VkPipelineMultisampleStateCreateInfo multisampleState = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampleState.rasterizationSamples = ms ? (VkSampleCountFlagBits)ms->sampleCount : VK_SAMPLE_COUNT_1_BIT;

    if (graphicsPipelineDesc.multisample) {
      multisampleState.sampleShadingEnable = false;
      multisampleState.minSampleShading = 0.0f;
      multisampleState.pSampleMask = ms->sampleMask != 0 ? &ms->sampleMask : nullptr;
      multisampleState.alphaToCoverageEnable = ms->alphaToCoverage;
      multisampleState.alphaToOneEnable = false;
      PNEXT_CHAIN_DECLARE(multisampleState.pNext);

      if (ms->sampleLocations) {
        sampleLocationsState.sampleLocationsEnable = VK_TRUE;

        PNEXT_CHAIN_APPEND_STRUCT(sampleLocationsState);
      }
    }

    // Rasterization
    const RasterizationInfo &r = graphicsPipelineDesc.rasterization;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
    };
    rasterizationState.depthClampEnable = r.depthClamp;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE; // TODO: D3D doesn't have this
    rasterizationState.polygonMode = getVulkanPolygonMode(r.fillMode);
    rasterizationState.cullMode = getVulkanCullMode(r.cullMode);
    rasterizationState.frontFace = r.frontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.depthBiasEnable = depthBiasEnabled(r.depthBias) ? VK_TRUE : VK_FALSE;
    rasterizationState.depthBiasConstantFactor = r.depthBias.constant;
    rasterizationState.depthBiasClamp = r.depthBias.clamp;
    rasterizationState.depthBiasSlopeFactor = r.depthBias.slope;
    rasterizationState.lineWidth = 1.0f;
    PNEXT_CHAIN_DECLARE(rasterizationState.pNext);

    VkPipelineRasterizationConservativeStateCreateInfoEXT consetvativeRasterizationState = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT
    };
    if (r.conservativeRaster) {
      consetvativeRasterizationState.conservativeRasterizationMode =
        VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
      consetvativeRasterizationState.extraPrimitiveOverestimationSize = 0.0f;

      PNEXT_CHAIN_APPEND_STRUCT(consetvativeRasterizationState);
    }

    VkPipelineRasterizationLineStateCreateInfoKHR lineState = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_KHR
    };
    if (r.lineSmoothing) {
      lineState.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_KHR;
      PNEXT_CHAIN_APPEND_STRUCT(lineState);
    }

    depthBias = r.depthBias;

    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    if (!device.getInfo().features.extendedDynamicState) {
      viewportState.viewportCount = device.getInfo().viewport.maxNum;
      viewportState.scissorCount = device.getInfo().viewport.maxNum;
    }

    // Depth-stencil
    const DepthAttachmentDesc &da = graphicsPipelineDesc.outputMerger.depth;
    const StencilAttachmentDesc &sa = graphicsPipelineDesc.outputMerger.stencil;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
    };
    depthStencilState.depthTestEnable = da.compareOp != CompareOp::None;
    depthStencilState.depthWriteEnable = da.write;
    depthStencilState.depthCompareOp = getVulkanCompareOps(da.compareOp);
    depthStencilState.depthBoundsTestEnable = da.boundsTest;
    depthStencilState.stencilTestEnable = (sa.front.compareOp == CompareOp::None && sa.back.compareOp ==
                                            CompareOp::None)
                                            ? VK_FALSE
                                            : VK_TRUE;
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;

    depthStencilState.front.failOp = getVulkanStencilOps(sa.front.failOp);
    depthStencilState.front.passOp = getVulkanStencilOps(sa.front.passOp);
    depthStencilState.front.depthFailOp = getVulkanStencilOps(sa.front.depthFailOp);
    depthStencilState.front.compareOp = getVulkanCompareOps(sa.front.compareOp);
    depthStencilState.front.compareMask = sa.front.compareMask;
    depthStencilState.front.writeMask = sa.front.writeMask;

    depthStencilState.back.failOp = getVulkanStencilOps(sa.back.failOp);
    depthStencilState.back.passOp = getVulkanStencilOps(sa.back.passOp);
    depthStencilState.back.depthFailOp = getVulkanStencilOps(sa.back.depthFailOp);
    depthStencilState.back.compareOp = getVulkanCompareOps(sa.back.compareOp);
    depthStencilState.back.compareMask = sa.back.compareMask;
    depthStencilState.back.writeMask = sa.back.writeMask;

    // Blending
    const OutputMergerInfo &om = graphicsPipelineDesc.outputMerger;
    std::vector<VkPipelineColorBlendAttachmentState> scratch(om.colorCount);

    VkPipelineColorBlendStateCreateInfo colorBlendState = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendState.logicOpEnable = om.logicOp != LogicOp::None ? VK_TRUE : VK_FALSE;
    colorBlendState.logicOp = getVulkanLogicOp(om.logicOp);
    colorBlendState.attachmentCount = om.colorCount;
    colorBlendState.pAttachments = scratch.data();

    bool isConstantColorReferenced = false;
    VkPipelineColorBlendAttachmentState *attachments = const_cast<VkPipelineColorBlendAttachmentState*>(colorBlendState.
      pAttachments);
    for (uint32_t i = 0; i < om.colorCount; i++) {
      const ColorAttachmentInfo &attachmentDesc = om.colors[i];

      attachments[i] = {
        VkBool32(attachmentDesc.blendEnabled),
        getVulkanBlendFactor(attachmentDesc.colorBlend.srcFactor),
        getVulkanBlendFactor(attachmentDesc.colorBlend.dstFactor),
        getVulkanBlendOp(attachmentDesc.colorBlend.op),
        getVulkanBlendFactor(attachmentDesc.alphaBlend.srcFactor),
        getVulkanBlendFactor(attachmentDesc.alphaBlend.dstFactor),
        getVulkanBlendOp(attachmentDesc.alphaBlend.op),
        getVulkanColorComponent(attachmentDesc.colorWriteMask),
      };

      if (constantColorReferenced(attachmentDesc.colorBlend.srcFactor) ||
        constantColorReferenced(attachmentDesc.colorBlend.dstFactor) ||
        constantColorReferenced(attachmentDesc.alphaBlend.srcFactor) || constantColorReferenced(
          attachmentDesc.alphaBlend.dstFactor))
        isConstantColorReferenced = true;
    }

    // Formats
    const FormatProperties &depthStencilFormatProps = getFormatProperties(om.depthStencilFormat);

    std::vector<VkFormat> colorFormats(om.colorCount);
    for (uint32_t i = 0; i < om.colorCount; i++)
      colorFormats[i] = getVulkanFormat(om.colors[i].format);

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    pipelineRenderingCreateInfo.viewMask = om.viewMask;
    pipelineRenderingCreateInfo.colorAttachmentCount = om.colorCount;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = colorFormats.data();
    pipelineRenderingCreateInfo.depthAttachmentFormat = getVulkanFormat(om.depthStencilFormat);
    pipelineRenderingCreateInfo.stencilAttachmentFormat = depthStencilFormatProps.isStencil
                                                            ? getVulkanFormat(om.depthStencilFormat)
                                                            : VK_FORMAT_UNDEFINED;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    if (!device.getVulkanFeatures().dynamicRendering) {
      RenderPassInfo renderPassDesc;
      renderPassDesc.viewMask = om.viewMask;
      fillRenderPassInputAttachmentCompatibilityIndices(renderPassDesc.inputAttachmentIndices, graphicsPipelineDesc);

      VkSampleCountFlagBits sampleNum = ms ? (VkSampleCountFlagBits)ms->sampleCount : VK_SAMPLE_COUNT_1_BIT;
      for (uint32_t i = 0; i < om.colorCount; i++) {
        RenderPassAttachmentInfo &color = renderPassDesc.colors.emplace_back();
        color.format = colorFormats[i];
        color.sampleNum = sampleNum;
        color.layout = hasRenderPassInputAttachmentIndex(renderPassDesc.inputAttachmentIndices, i)
                         ? VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ
                         : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      }

      VkFormat depthStencilFormat = getVulkanFormat(om.depthStencilFormat);
      if (depthStencilFormat != VK_FORMAT_UNDEFINED) {
        renderPassDesc.hasDepth = true;
        renderPassDesc.depth.format = depthStencilFormat;
        renderPassDesc.depth.sampleNum = sampleNum;
        renderPassDesc.depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        if (depthStencilFormatProps.isStencil) {
          renderPassDesc.hasStencil = true;
          renderPassDesc.stencil = renderPassDesc.depth;
        }
      }

      if (r.shadingRate) {
        renderPassDesc.hasShadingRate = true;
        renderPassDesc.shadingRate.format = VK_FORMAT_R8_UINT;
        renderPassDesc.shadingRate.sampleNum = VK_SAMPLE_COUNT_1_BIT;
        renderPassDesc.shadingRate.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        renderPassDesc.shadingRate.layout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
      }

      renderPass = device.getOrCreateRenderPass(renderPassDesc);
      if (!renderPass)
        return Result::Failure;
    }

    // Dynamic state
    uint32_t dynamicStateNum = 0;
    std::array<VkDynamicState, 16> dynamicStates = {};
    if (device.getInfo().features.extendedDynamicState) {
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT;
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT;
    }
    else {
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_VIEWPORT;
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_SCISSOR;
    }

    if (vi && device.getInfo().features.extendedDynamicState)
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE;
    if (rasterizationState.depthBiasEnable)
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_DEPTH_BIAS;
    if (depthStencilState.depthBoundsTestEnable)
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
    if (depthStencilState.stencilTestEnable)
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
    if (sampleLocationsState.sampleLocationsEnable)
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT;
    if (isConstantColorReferenced)
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
    if (r.shadingRate)
      dynamicStates[dynamicStateNum++] = VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR;

    VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicState.dynamicStateCount = dynamicStateNum;
    dynamicState.pDynamicStates = dynamicStates.data();

    // Create
    VkPipelineCreateFlags flags = 0;
    if (r.shadingRate && device.getVulkanFeatures().dynamicRendering)
      flags |= VK_PIPELINE_CREATE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    if ((graphicsPipelineDesc.flags & GraphicsPipelineBits::FailOnCacheMiss) && device.getInfo().features.
      pipelineCacheControl)
      flags |= VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;

    const VulkanPipelineLayout &pipelineLayoutVK = *(VulkanPipelineLayout*)graphicsPipelineDesc.pipelineLayout;

    VkGraphicsPipelineCreateInfo info = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      nullptr,
      flags,
      graphicsPipelineDesc.shaderCount,
      stages.data(),
      &vertexInputState,
      &inputAssemblyState,
      &tessellationState,
      &viewportState,
      &rasterizationState,
      &multisampleState,
      &depthStencilState,
      &colorBlendState,
      &dynamicState,
      pipelineLayoutVK,
      renderPass,
      0,
      VK_NULL_HANDLE,
      -1,
    };

    PNEXT_CHAIN_SET(info.pNext);
    if (device.getVulkanFeatures().dynamicRendering)
      PNEXT_CHAIN_APPEND_STRUCT(pipelineRenderingCreateInfo);

    VkPipelineRobustnessCreateInfoEXT robustnessInfo = {VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO_EXT};
    if (fillPipelineRobustness(device, graphicsPipelineDesc.robustness, robustnessInfo))
      PNEXT_CHAIN_APPEND_STRUCT(robustnessInfo);

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    if (graphicsPipelineDesc.cache)
      pipelineCache = *(VulkanPipelineCache*)graphicsPipelineDesc.cache;
    VkResult vkResult = vkCreateGraphicsPipelines(device, pipelineCache, 1, &info, device.getAllocationCallbacks(),
                                                  &pipeline);

    for (size_t i = 0; i < graphicsPipelineDesc.shaderCount; i++)
      vkDestroyShaderModule(device, modules[i], device.getAllocationCallbacks());

    if (vkResult == VK_PIPELINE_COMPILE_REQUIRED)
      return Result::Failure;

    VULKAN_CHECK(vkResult);

    return Result::Success;
  }

  Result VulkanPipeline::create(const ComputePipelineInfo &computePipelineDesc) {
    bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

    const VulkanPipelineLayout &pipelineLayoutVK = *(VulkanPipelineLayout*)computePipelineDesc.pipelineLayout;

    const VkShaderModuleCreateInfo moduleInfo = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      nullptr,
      (VkShaderModuleCreateFlags)0,
      (size_t)computePipelineDesc.shader.size,
      (const uint32_t*)computePipelineDesc.shader.bytecode,
    };

    VkShaderModule module = VK_NULL_HANDLE;
    VULKAN_CHECK(vkCreateShaderModule(device, &moduleInfo, device.getAllocationCallbacks(), &module));

    VkPipelineShaderStageCreateInfo stage = {
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      nullptr,
      (VkPipelineShaderStageCreateFlags)0,
      VK_SHADER_STAGE_COMPUTE_BIT,
      module,
      computePipelineDesc.shader.entryPointName ? computePipelineDesc.shader.entryPointName : "main",
      nullptr,
    };

    VkPipelineCreateFlags computeFlags = 0;
    if ((computePipelineDesc.flags & ComputePipelineBits::FailOnCacheMiss) && device.getInfo().features.
      pipelineCacheControl)
      computeFlags |= VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;

    VkComputePipelineCreateInfo info = {
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      nullptr,
      computeFlags,
      stage,
      pipelineLayoutVK,
      VK_NULL_HANDLE,
      -1,
    };

    VkPipelineRobustnessCreateInfoEXT robustnessInfo = {VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO_EXT};
    if (fillPipelineRobustness(device, computePipelineDesc.robustness, robustnessInfo))
      info.pNext = &robustnessInfo;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    if (computePipelineDesc.cache)
      pipelineCache = *(VulkanPipelineCache*)computePipelineDesc.cache;
    VkResult vkResult = vkCreateComputePipelines(
      device, pipelineCache, 1, &info, device.getAllocationCallbacks(), &pipeline
    );

    vkDestroyShaderModule(device, module, device.getAllocationCallbacks());

    if (vkResult == VK_PIPELINE_COMPILE_REQUIRED)
      return Result::Failure;

    VULKAN_CHECK(vkResult);

    return Result::Success;
  }

  Result VulkanPipeline::setupShaderStage(
    VkPipelineShaderStageCreateInfo &stage,
    const ShaderInfo &shaderDesc,
    VkShaderModule &module
  ) {
    const VkShaderModuleCreateInfo moduleInfo = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      nullptr,
      (VkShaderModuleCreateFlags)0,
      (size_t)shaderDesc.size,
      (const uint32_t*)shaderDesc.bytecode,
    };

    VULKAN_CHECK(vkCreateShaderModule(device, &moduleInfo, device.getAllocationCallbacks(), &module));

    stage = {
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      nullptr,
      (VkPipelineShaderStageCreateFlags)0,
      (VkShaderStageFlagBits)getVulkanShaderStageFlags(shaderDesc.stage),
      module,
      nullptr,
      nullptr,
    };

    return Result::Success;
  }

  Result VulkanPipeline::writeShaderGroupIdentifiers(uint32_t baseShaderGroupIndex, uint32_t shaderGroupNum, void *dst) const {
    const size_t dataSize = (size_t)shaderGroupNum * device.getInfo().shaderStage.rayTracing.shaderGroupIdentifierSize;
    VULKAN_CHECK(vkGetRayTracingShaderGroupHandlesKHR(
      device, pipeline, baseShaderGroupIndex, shaderGroupNum, dataSize, dst)
    );
    return Result::Success;
  }
}
