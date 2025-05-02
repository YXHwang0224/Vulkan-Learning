#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {

	class Buffer {
	

	public:
		using Ptr = std::shared_ptr<Buffer>;
		static Ptr create(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags property) {
			return std::make_shared<Buffer>(device, size, usage, property);
		}
	public:
		static Ptr createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData);
		static Ptr createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData);
		static Ptr createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData = nullptr);

	public:
		Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags property);

		~Buffer();

		/*
		1.对于哪些我们经常要更改的buffer（Host Visible），我们使用内存Mapping的形式，直接在Host端进行管理
		2.对于哪些Host端调用少而Device端调用频繁的buffer（LogicOptimal），那么我们就创建中间的StageBuffer，先复制到StageBuffer，再拷贝到目标Buffer
		*/
		void updateBufferByMap(void* data, size_t size);

		void updateBufferStage(void* data, size_t size);

		void copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);

		[[nodiscard]] auto getBuffer() {
			return myBuffer;
		}

		[[nodiscard]] VkDescriptorBufferInfo& getBufferInfo() {
			return myBufferInfo;
		}

	private:
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	private:
		VkBuffer myBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory myBufferMemory{ VK_NULL_HANDLE };
		Device::Ptr myDevice{ nullptr };
		VkDescriptorBufferInfo myBufferInfo{};
	};
}