#include <array>
#include <SDL3/SDL_vulkan.h>
#include <core/application/logger.hpp>
#include "vulkan_swapchain.hpp"
#include "vulkan_rendering_device.hpp"
#include "vulkan_context.hpp"
#include "volk.h"

namespace Core::Graphics {
  VulkanSwapChain::VulkanSwapChain(
    const VkInstance &instance,
    RefCountedPtr<VulkanDevice> device,
    SDL_Window *const window,
    std::uint32_t width,
    std::uint32_t height
  ) : window(window), instance(instance), device(device) {
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
      LOG_CORE_ERROR("Failed to create Vulkan window surface");
      return;
    }

    VkPhysicalDevice physicalDevice = device->getPhysicalDevice()->physicalDevice;

    // Get list of supported surface formats
    uint32_t formatCount;
    VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
    if (formatCount == 0) {
      LOG_CORE_ERROR("No supported surface formats found");
      return;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data()));

  	this->surfaceFormat.format = VK_FORMAT_B8G8R8A8_SRGB ;

    // Creating swap chain
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice,
      surface,
      &surfaceCapabilities
    );

    VkExtent2D swapChainExtents {surfaceCapabilities.currentExtent};
    if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
      swapChainExtents = {.width = width, .height = height};

    VkSwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.minImageCount =  VulkanRenderingDevice::framesInFlight;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageExtent = {.width = swapChainExtents.width, .height = swapChainExtents.height};
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    VULKAN_CHECK(vkCreateSwapchainKHR(device->logicalDevice, &swapChainCreateInfo, nullptr, &swapChain));

    std::uint32_t imageCount = VulkanRenderingDevice::framesInFlight;
    VULKAN_CHECK(vkGetSwapchainImagesKHR(device->logicalDevice, swapChain, &imageCount, nullptr));
    images.resize(imageCount);
    VULKAN_CHECK(vkGetSwapchainImagesKHR(device->logicalDevice, swapChain, &imageCount, images.data()));
    imageViews.resize(imageCount);
  	for (auto i = 0; i < imageCount; i++) {
  		VkImageViewCreateInfo viewCI {
  			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
  			.image = images[i], .viewType = VK_IMAGE_VIEW_TYPE_2D,
  			.format = this->surfaceFormat.format,
  			.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
  		};
  		VULKAN_CHECK(vkCreateImageView(device->logicalDevice, &viewCI, nullptr, &imageViews[i]));
  	}

    // Synchronization primitives
    imageAvailableSemaphores.resize(VulkanRenderingDevice::framesInFlight);
    renderFinishedSemaphores.resize(VulkanRenderingDevice::framesInFlight);

    VkSemaphoreCreateInfo semaphoreCreateInfo {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (size_t i = 0; i < VulkanRenderingDevice::framesInFlight; i++) {
      VULKAN_CHECK(vkCreateSemaphore(device->logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]));
      VULKAN_CHECK(vkCreateSemaphore(device->logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]));
    }

    VkFenceCreateInfo fenceCreateInfo {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    waitFences.resize(VulkanRenderingDevice::framesInFlight);
    for (auto& fence: waitFences) {
      VULKAN_CHECK(vkCreateFence(device->logicalDevice, &fenceCreateInfo, nullptr, &fence));
    }

  	commandBuffers.resize(VulkanRenderingDevice::framesInFlight);
  	for (auto &cmdBuffer: commandBuffers) {
  		VkCommandPoolCreateInfo poolInfo {};
  		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = device->getPhysicalDevice()->getGraphicsQueueIndex();

  		VULKAN_CHECK(vkCreateCommandPool(device->logicalDevice, &poolInfo, nullptr, &cmdBuffer.commandPool));

  		VkCommandBufferAllocateInfo allocInfo {};
  		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  		allocInfo.commandPool = cmdBuffer.commandPool;
  		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  		allocInfo.commandBufferCount = 1;

			VULKAN_CHECK(vkAllocateCommandBuffers(device->logicalDevice, &allocInfo, &cmdBuffer.commandBuffer));
  	}
  }

  void VulkanSwapChain::destroy() {
    vkDeviceWaitIdle(device->logicalDevice);

    for (VkSemaphore semaphore: imageAvailableSemaphores) {
      vkDestroySemaphore(device->logicalDevice, semaphore, nullptr);
    }

    for (VkSemaphore semaphore: renderFinishedSemaphores) {
      vkDestroySemaphore(device->logicalDevice, semaphore, nullptr);
    }

    for (VkFence fence: waitFences) {
      vkDestroyFence(device->logicalDevice, fence, nullptr);
    }

  	for (VkImageView imageView: imageViews) {
  		vkDestroyImageView(device->logicalDevice, imageView, nullptr);
  	}

  	for (SwapChainCommandBuffer commandBuffer: commandBuffers) {
  		vkFreeCommandBuffers(device->logicalDevice, commandBuffer.commandPool, 1, &commandBuffer.commandBuffer);
  		vkDestroyCommandPool(device->logicalDevice, commandBuffer.commandPool, nullptr);
  	}

    vkDestroySwapchainKHR(device->logicalDevice, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDeviceWaitIdle(device->logicalDevice);

  	vkDestroyPipelineLayout(device->logicalDevice, pipelineLayout, nullptr);
  	vkDestroyPipeline(device->logicalDevice, pipeline, nullptr);
  }

  void VulkanSwapChain::present(VkPipeline pipeline, VkPipelineLayout pipelineLayout) const {
  	this->pipeline = pipeline;
  	this->pipelineLayout = pipelineLayout;
    VULKAN_CHECK(vkWaitForFences(device->logicalDevice, 1, &waitFences[currentFrameIndex], true, UINT64_MAX));
    VULKAN_CHECK(vkResetFences(device->logicalDevice, 1, &waitFences[currentFrameIndex]));

    VULKAN_CHECK(vkAcquireNextImageKHR(device->logicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrameIndex], VK_NULL_HANDLE, &imageIndex));

    SwapChainCommandBuffer commandBuffer = commandBuffers[currentFrameIndex];
    VULKAN_CHECK(vkResetCommandBuffer(commandBuffer.commandBuffer, 0));

  	VkCommandBufferBeginInfo commandBufferBeginInfo {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;
  	VULKAN_CHECK(vkBeginCommandBuffer(commandBuffer.commandBuffer, &commandBufferBeginInfo));

  	VkImageMemoryBarrier2 outputBarrier{
  		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.image = images[imageIndex],
			.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
  	};

  	VkDependencyInfo barrierDependencyInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &outputBarrier };
  	vkCmdPipelineBarrier2(commandBuffer.commandBuffer, &barrierDependencyInfo);

  	float flashValue = static_cast<float>(flash);
    VkRenderingAttachmentInfo colorAttachmentInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = imageViews[imageIndex],
      .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue{.color{0.5f, 0.8f, 0.5f, 1.0f }}
    };
    VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea{.extent{.width = static_cast<uint32_t>(surfaceCapabilities.currentExtent.width), .height = static_cast<uint32_t>(surfaceCapabilities.currentExtent.height) }},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentInfo,
    };

    vkCmdBeginRendering(commandBuffer.commandBuffer, &renderingInfo);
  	vkCmdBindPipeline(commandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport vp{ .width = static_cast<float>(surfaceCapabilities.currentExtent.width), .height = static_cast<float>(surfaceCapabilities.currentExtent.height), .minDepth = 0.0f, .maxDepth = 1.0f};
		vkCmdSetViewport(commandBuffer.commandBuffer, 0, 1, &vp);
  	VkRect2D scissor { .extent{ .width = static_cast<uint32_t>(surfaceCapabilities.currentExtent.width), .height = static_cast<uint32_t>(surfaceCapabilities.currentExtent.height) } };
		vkCmdSetScissor(commandBuffer.commandBuffer, 0, 1, &scissor);

  	vkCmdDraw(commandBuffer.commandBuffer, 3, 1, 0, 0);
    vkCmdEndRendering(commandBuffer.commandBuffer);

  	VkImageMemoryBarrier2 barrierPresent{
  		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT	,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.image = images[imageIndex],
			.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1, .layerCount = 1 }
  	};

  	VkDependencyInfo barrierPresentDependencyInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrierPresent };
		vkCmdPipelineBarrier2(commandBuffer.commandBuffer, &barrierPresentDependencyInfo);
  	VULKAN_CHECK(vkEndCommandBuffer(commandBuffer.commandBuffer));

  	// Submit to graphics queue
		VkSemaphoreSubmitInfo waitSemaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .semaphore = imageAvailableSemaphores[currentFrameIndex], .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkCommandBufferSubmitInfo commandBufferSubmitInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = commandBuffer.commandBuffer };
		VkSemaphoreSubmitInfo signalSemaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .semaphore = renderFinishedSemaphores[imageIndex], .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo2 submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = 1,
			.pWaitSemaphoreInfos = &waitSemaphoreInfo,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &commandBufferSubmitInfo,
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos = &signalSemaphoreInfo,
		};
		VULKAN_CHECK(vkQueueSubmit2(device->getGraphicsQueue(), 1, &submitInfo, waitFences[currentFrameIndex]));

	  currentFrameIndex = (currentFrameIndex + 1) % VulkanRenderingDevice::framesInFlight;
		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &renderFinishedSemaphores[imageIndex],
			.swapchainCount = 1,
			.pSwapchains = &swapChain,
			.pImageIndices = &imageIndex
		};
		VULKAN_CHECK(vkQueuePresentKHR(device->getGraphicsQueue(), &presentInfo));
  }
}
