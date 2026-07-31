#include <array>
#include <core/memory.hpp>
#include "vulkan_shader.hpp"
#include "vulkan_rendering_device.hpp"

namespace Core::Graphics {
  VulkanRenderingDevice::VulkanRenderingDevice(Backend backend, const char *appName, const DisplayInfo &displayInfo) :
  backend(backend), appName(appName), displayInfo(displayInfo), vulkanContext(appName) {
    RefCountedPtr<VulkanShader> shader = RefCountedPtr<VulkanShader>::create(vulkanContext.getDevice(), "resources/shaders/shader.spv");
    //
    // VkDescriptorBindingFlags descVariableFlag{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
    // VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlags{
    //   .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
    //   .bindingCount = 1,
    //   .pBindingFlags = &descVariableFlag
    // };
    // VkDescriptorSetLayoutBinding descLayoutBindingTex{
    //   .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    //   .descriptorCount = 1,
    //   .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    // };
    // VkDescriptorSetLayoutCreateInfo descLayoutTexCI{
    //   .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    //   .pNext = &descBindingFlags,
    //   .bindingCount = 1,
    //   .pBindings = &descLayoutBindingTex
    // };
    // VULKAN_CHECK(vkCreateDescriptorSetLayout(vulkanContext.getDevice()->logicalDevice, &descLayoutTexCI, nullptr, &descriptorSetLayoutTex));

    // VkPushConstantRange pushConstantRange {};
    // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // pushConstantRange.size = sizeof(VkDeviceAddress);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    VULKAN_CHECK(vkCreatePipelineLayout(
      vulkanContext.getDevice()->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout
    ));

    VkPipelineColorBlendAttachmentState blendAttachment {};
    blendAttachment.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo colorBlendState {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &blendAttachment
    };

    VkPipelineVertexInputStateCreateInfo vertexInput {};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 0;
    vertexInput.pVertexBindingDescriptions = nullptr;
    vertexInput.vertexAttributeDescriptionCount = 0;
    vertexInput.pVertexAttributeDescriptions = nullptr;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo {};
    depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo {};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_TRUE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    // rasterizerCreateInfo.polygonMode = translateFillMode(rasterizerDesc.fillMode);
    // rasterizerCreateInfo.cullMode = translateCullMode(rasterizerDesc.cullMode);
    // rasterizerCreateInfo.frontFace = translateFrontFaceMode(rasterizerDesc.frontFace);
    // rasterizerCreateInfo.depthBiasEnable = (rasterizerDesc.depthBias == 0) ? VK_FALSE : VK_TRUE;
    // rasterizerCreateInfo.depthBiasConstantFactor = (float)rasterizerDesc.depthBias;
    // rasterizerCreateInfo.depthBiasClamp = rasterizerDesc.depthBiasClamp;
    // rasterizerCreateInfo.depthBiasSlopeFactor = rasterizerDesc.slopeScaledDepthBias;
    rasterizerCreateInfo.lineWidth = 1.0f;

    // VkPipelineMultisampleStateCreateInfo multisampleState {};
    // multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    // multisampleStateCreateInfo.alphaToCoverageEnable = desc.multisample.alphaToCoverageEnable;
    // multisampleStateCreateInfo.alphaToOneEnable = desc.multisample.alphaToOneEnable;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    constexpr std::array dynamicStates {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_STENCIL_REFERENCE,
      VK_DYNAMIC_STATE_BLEND_CONSTANTS
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    // viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    // viewportState.pScissors = &scissor;

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo {};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;

    std::array formats {
      VK_FORMAT_B8G8R8A8_SRGB
    };

    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = formats.data();
    pipelineRenderingCreateInfo.depthAttachmentFormat = vulkanContext.getDevice()->getPhysicalDevice()->getDepthFormat();

    VkGraphicsPipelineCreateInfo pipelineCreateInfo {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
    pipelineCreateInfo.stageCount = shader->getShaderStages().size();
    pipelineCreateInfo.pStages = shader->getShaderStages().data();
    pipelineCreateInfo.pVertexInputState = &vertexInput;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

    VULKAN_CHECK(vkCreateGraphicsPipelines(
      vulkanContext.getDevice()->logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline
    ));
  }

  VulkanRenderingDevice::~VulkanRenderingDevice() {
    // vkDestroyDescriptorSetLayout(vulkanContext.getDevice()->logicalDevice, descriptorSetLayoutTex, nullptr);
  }

  void VulkanRenderingDevice::aquireNextImage() {
    for (const auto &[id, swapChain]: vulkanContext.getSwapChains()) {
      swapChain.present(pipeline, pipelineLayout);
    }
  }
}
