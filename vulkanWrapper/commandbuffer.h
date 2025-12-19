#pragma once

#include "../base.h"
#include "commandpool.h"
#include "device.h"

namespace FF::Wrapper {

	//首先，CommandBuffer本质是分配内存空间，每个指令缓冲需要指定其存在的指令池以及其分级（primary还是secondary）
	//CommandBuffer中定义了指令从开始到结束的6个函数：begin，beginRenderPass，bindPipeline，draw，endRenderPass，end

	class CommandBuffer {
	public:
		using Ptr = std::shared_ptr<CommandBuffer>;
		static Ptr create(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary = false) {
			return std::make_shared<CommandBuffer>(device, commandPool, asSecondary);
		}

		CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary = false);

		~CommandBuffer();

		//begin -> beginRenderPass 绑定各类实际数据 -> endRenderPass -> end

		//VkCommandBufferUsageFlags:
		//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT：这个命令只会被提交一次
		//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT：这个命令缓冲是一个二级缓冲
		//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_BIT：这个命令已经被提交了，提交期间可以再次提交

		//VkCommandBufferInheritanceInfo：如果本命令缓冲是一个二级缓冲，那么这个结构体继承了其主命令的信息
		void begin(VkCommandBufferUsageFlags flag = 0, const VkCommandBufferInheritanceInfo& inheritance = {});

		//VkSubpassContents:
		//VK_SUBPASS_CONTENTS_INLINEL:渲染指令会被记录在命令缓冲，本命令缓冲肯定就是主命令缓冲
		//VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS:渲染指令存放在二级缓冲中，被主命令缓冲调用beginRenderPass的时候并且使用二级命令缓冲的情况下使用
		void beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, const VkSubpassContents& subPassContents = VK_SUBPASS_CONTENTS_INLINE);

		void bindGraphicPipeline(const VkPipeline& pipeline);

		void bindVertexBuffer(const std::vector<VkBuffer> &buffers);

		void bindIndexBuffer(const VkBuffer& buffer);

		void bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet& descriptorSet);

		void draw(size_t vertexCount);
		void drawIndex(size_t indexCount);

		void endRenderPass();

		void end();

		void copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos);

		void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height);

		void submitSync(VkQueue queue, VkFence fence = VK_NULL_HANDLE);

		void transferImageLayout(const VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

		[[nodiscard]] auto getCommandBuffer() {
			return myCommandBuffer;
		}
	private:
		VkCommandBuffer myCommandBuffer{ VK_NULL_HANDLE };
		Device::Ptr mydevice{ nullptr };
		CommandPool::Ptr myCommandPool{ nullptr };
	};

}