#include "vulkan_backend.hpp"

namespace {
  using namespace Core::RHI;

  constexpr VkImageUsageFlags getImageUsageFlags(TextureUsageBits textureUsageBits) {
    VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (textureUsageBits & TextureUsageBits::ShaderResource)
      flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

    if (textureUsageBits & TextureUsageBits::ShaderResourceStorage)
      flags |= VK_IMAGE_USAGE_STORAGE_BIT;

    if (textureUsageBits & TextureUsageBits::ColorAttachment)
      flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (textureUsageBits & TextureUsageBits::DepthStencilAttachment)
      flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (textureUsageBits & TextureUsageBits::ShadingRateAttachment)
      flags |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;

    if (textureUsageBits & TextureUsageBits::InputAttachment)
      flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    return flags;
  }
}

namespace Core::RHI {
  VulkanTexture::VulkanTexture(VulkanDevice &device): device(device) {
  }

  VulkanTexture::~VulkanTexture() {
    if (vmaAllocation)
      vmaDestroyImage(device.getVma(), image, vmaAllocation);
    else
      vkDestroyImage(device, image, device.getAllocationCallbacks());
  }

  Result VulkanTexture::create(const TextureInfo &textureInfo) {
    info.height = std::max(textureInfo.height, static_cast<uint16_t>(1));
    info.depth = std::max(textureInfo.depth, static_cast<uint16_t>(1));
    info.mipCount = std::max(textureInfo.mipCount, static_cast<uint16_t>(1));
    info.layerCount = std::max(textureInfo.layerCount, static_cast<uint16_t>(1));
    info.sampleCount = std::max(textureInfo.sampleCount, static_cast<uint8_t>(1));

    VkImageCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    const FormatProperties& formatProps = getFormatProperties(textureInfo.format);

    VkImageCreateFlags flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT // typeless (basic)
        | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT                      // typeless (advanced)
        | VK_IMAGE_CREATE_ALIAS_BIT;                              // matches https://learn.microsoft.com/en-us/windows/win32/direct3d12/memory-aliasing-and-data-inheritance#data-inheritance

    if (formatProps.blockWidth > 1 && (textureInfo.usage & TextureUsageBits::ShaderResourceStorage))
        flags |= VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT; // format can be used to create a view with an uncompressed format (1 texel covers 1 block)
    if (textureInfo.layerCount >= 6 && textureInfo.width == textureInfo.height)
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // allow cube maps
    if (textureInfo.dimension == TextureDimension::Dimension3D)
        flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT; // allow 3D demotion to a set of layers // TODO: hook up "VK_EXT_image_2d_view_of_3d"?
    if (device.getInfo().tiers.sampleLocations && formatProps.isDepth)
        flags |= VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT;

    createInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO}; // should be already set
    createInfo.flags = flags;
    createInfo.imageType = textureDimensionToVulkanImageType(textureInfo.dimension);
    createInfo.format = formatToVulkanFormat(textureInfo.format, true);
    createInfo.extent.width = textureInfo.width;
    createInfo.extent.height = std::max(textureInfo.height, static_cast<uint16_t>(1));
    createInfo.extent.depth = std::max(textureInfo.depth, static_cast<uint16_t>(1));
    createInfo.mipLevels = std::max(textureInfo.mipCount, static_cast<uint16_t>(1));
    createInfo.arrayLayers = std::max(textureInfo.layerCount, static_cast<uint16_t>(1));
    createInfo.samples = (VkSampleCountFlagBits)std::max(textureInfo.sampleCount, static_cast<uint8_t>(1));
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = getImageUsageFlags(textureInfo.usage);
    // TODO: uncomment and implement
    // createInfo.sharingMode = (m_NumActiveFamilyIndices <= 1 || textureInfo.sharingMode == SharingMode::Exclusive) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    // createInfo.queueFamilyIndexCount = m_NumActiveFamilyIndices;
    // createInfo.pQueueFamilyIndices = m_ActiveQueueFamilyIndices.data();
    createInfo.initialLayout = device.memoryZeroInitializationEnabled() ? VK_IMAGE_LAYOUT_ZERO_INITIALIZED_EXT : VK_IMAGE_LAYOUT_UNDEFINED;

    VULKAN_CHECK(vkCreateImage(device, &createInfo, device.getAllocationCallbacks(), &image));
    return Result::Success;
  }

  void VulkanTexture::getMemoryInfo(MemoryLocation memoryLocation, MemoryInfo &memoryInfo) const {
    VkMemoryDedicatedRequirements dedicatedRequirements = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS};

    VkMemoryRequirements2 requirements = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};
    requirements.pNext = &dedicatedRequirements;

    VkImageMemoryRequirementsInfo2 imageMemoryRequirements = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
    imageMemoryRequirements.image = image;

    vkGetImageMemoryRequirements2(device, &imageMemoryRequirements, &requirements);
    memoryInfo = {};
    device.getMemoryInfo(memoryLocation, requirements.memoryRequirements, dedicatedRequirements, memoryInfo);
  }
}
