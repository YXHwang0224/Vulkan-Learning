#include "commandbuffer.h"

namespace FF::Wrapper {
	CommandBuffer::CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary) {
		mydevice = device;
		myCommandPool = commandPool;

		//allocate一般为在GPU端分配内存的操作
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = myCommandPool->getCommandPool();
		allocInfo.level = asSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		if (vkAllocateCommandBuffers(mydevice->getDevice(), &allocInfo, &myCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error: failes to create commandBuffer");
		}
	}

	//会随着CommandPool析构而析构
	CommandBuffer::~CommandBuffer() {		
		if (myCommandBuffer != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(mydevice->getDevice(), myCommandPool->getCommandPool(), 1, &myCommandBuffer);
		}
	}	

	void CommandBuffer::begin(VkCommandBufferUsageFlags flag, 
		const VkCommandBufferInheritanceInfo& inheritance) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flag;
		beginInfo.pInheritanceInfo = &inheritance;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(myCommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to begin commandBuffer");
		}
	}

	void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo,
		const VkSubpassContents& subPassContents) {
		vkCmdBeginRenderPass(myCommandBuffer, &renderPassBeginInfo, subPassContents);
	}

	void CommandBuffer::bindGraphicPipeline(const VkPipeline& pipeline) {
		vkCmdBindPipeline(myCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void CommandBuffer::bindVertexBuffer(const std::vector<VkBuffer>& buffers) {
		std::vector<VkDeviceSize> offsets(buffers.size(), 0);	//表示对每个buffer我们都从头开始读数据
		
		vkCmdBindVertexBuffers(myCommandBuffer, 0, static_cast<uint32_t>(buffers.size()), buffers.data(),offsets.data());
	}

	void CommandBuffer::bindIndexBuffer(const VkBuffer& buffer) {
		vkCmdBindIndexBuffer(myCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet& descriptorSet) {
		vkCmdBindDescriptorSets(myCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
	}

	void CommandBuffer::draw(size_t vertexCount) {
		vkCmdDraw(myCommandBuffer, vertexCount, 1, 0, 0);
	}

	void CommandBuffer::drawIndex(size_t indexCount) {
		vkCmdDrawIndexed(myCommandBuffer, indexCount, 1, 0, 0, 0);
	}

	void CommandBuffer::endRenderPass() {
		vkCmdEndRenderPass(myCommandBuffer);
	}

	void CommandBuffer::end() {
		if (vkEndCommandBuffer(myCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to end commandBuffer");
		}
	}

	void CommandBuffer::copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos) {
		vkCmdCopyBuffer(myCommandBuffer, srcBuffer, dstBuffer, copyInfoCount, copyInfos.data());
	}

	void CommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height) {
		VkBufferImageCopy bufferCopyRegion{};
		bufferCopyRegion.bufferOffset = 0;

		bufferCopyRegion.bufferRowLength = 0;
		bufferCopyRegion.bufferImageHeight = 0;
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageOffset = { 0, 0, 0 };
		bufferCopyRegion.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(myCommandBuffer, srcBuffer, dstImage, dstImageLayout, 1, &bufferCopyRegion);
	}

	void CommandBuffer::submitSync(VkQueue queue, VkFence fence) {
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &myCommandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, fence);

		vkQueueWaitIdle(queue);
	}

	void CommandBuffer::transferImageLayout(
		const VkImageMemoryBarrier& imageMemoryBarrier, 
		VkPipelineStageFlags srcStageMask, 
		VkPipelineStageFlags dstStageMask
	) {
		vkCmdPipelineBarrier(
			myCommandBuffer, 
			srcStageMask, 
			dstStageMask, 
			0, 
			0, nullptr, 
			0, nullptr, 
			1, &imageMemoryBarrier
		);
	}
}