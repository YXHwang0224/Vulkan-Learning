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
		1.������Щ���Ǿ���Ҫ���ĵ�buffer��Host Visible��������ʹ���ڴ�Mapping����ʽ��ֱ����Host�˽��й���
		2.������ЩHost�˵����ٶ�Device�˵���Ƶ����buffer��LogicOptimal������ô���Ǿʹ����м��StageBuffer���ȸ��Ƶ�StageBuffer���ٿ�����Ŀ��Buffer
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