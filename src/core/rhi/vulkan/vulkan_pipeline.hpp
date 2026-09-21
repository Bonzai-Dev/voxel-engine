#pragma once
#include <core/rhi/rhi.hpp>

namespace Core::RHI {
  class VulkanPipeline final: public Pipeline {
    public:
      inline VulkanPipeline(VulkanDevice &device): device(device) {
      }

      ~VulkanPipeline() = default;

      inline operator VkPipeline() const {
        return pipeline;
      }

      inline VulkanDevice &getDevice() const {
        return device;
      }

      inline VkPipelineBindPoint getBindPoint() const {
        return bindPoint;
      }

      inline const DepthBiasInfo &getDepthBias() const {
        return depthBias;
      }

      Result create(const GraphicsPipelineInfo &graphicsPipelineDesc);
      Result create(const ComputePipelineInfo &computePipelineDesc);
      // Result create(const RayTracingPipelineInfo& rayTracingPipelineDesc);
      // Result create(const PipelineVKDesc& pipelineVKDesc);

      ENGINE_FORCE_INLINE Result writeShaderGroupIdentifiers(uint32_t baseShaderGroupIndex, uint32_t shaderGroupNum, void *dst) const;

    private:
      VulkanDevice &device;
      VkPipeline pipeline = VK_NULL_HANDLE;
      VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
      DepthBiasInfo depthBias = {};

      Result setupShaderStage(
        VkPipelineShaderStageCreateInfo &stage,
        const ShaderInfo &shaderDesc,
        VkShaderModule &module
      );
  };
}