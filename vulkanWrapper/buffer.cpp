#include "buffer.h"
#include "commandbuffer.h"

namespace FF::Wrapper {

	Buffer::Ptr Buffer::createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData) {
		//VK_BUFFER_USAGE_TRANSFER_DST_BIT表示其是一个被拷贝的目标点
		//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT表示其仅可在GPU可读
		auto buffer = Buffer::create(
			device, size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		buffer->updateBufferStage(pData, size);

		return buffer;
	}

	Buffer::Ptr Buffer::createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData) {
		auto buffer = Buffer::create(
			device, size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		buffer->updateBufferStage(pData, size);

		return buffer;
	}

	Buffer::Ptr Buffer::createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData) {
		auto buffer = Buffer::create(
			device, size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		if (pData != nullptr) {
			buffer->updateBufferStage(pData, size);
		}

		return buffer;
	}


	Buffer::Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
		myDevice = device;

		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		if (vkCreateBuffer(myDevice->getDevice(), &createInfo, nullptr, &myBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create buffer");
		}

		//创建显存信息
		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(myDevice->getDevice(), myBuffer, &memoryRequirements);

		//分配显存
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;

		//符合上述buffer要求的内存类型的ID集合
		allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(myDevice->getDevice(), &allocateInfo, nullptr, &myBufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to allocate memory");
		}

		//将显存与缓冲绑定
		vkBindBufferMemory(myDevice->getDevice(), myBuffer, myBufferMemory, 0);

		myBufferInfo.buffer = myBuffer;
		myBufferInfo.offset = 0;
		myBufferInfo.range = size;
	}

	Buffer::~Buffer() {
		if (myBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(myDevice->getDevice(), myBuffer, nullptr);
		}

		if (myBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(myDevice->getDevice(), myBufferMemory, nullptr);
		}
	}

	uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(myDevice->getPhysicalDevice(), &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
			//前面一部分是检验内存是否满足要求，后面一部分是检验是否满足特性需求
			if (typeFilter & (1 << i) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
				return i;
			}
		}

		throw std::runtime_error("Error: failed to the property memory type");
	}

	void Buffer::updateBufferByMap(void* data, size_t size) {
		void* memoryPtr = nullptr;

		vkMapMemory(myDevice->getDevice(), myBufferMemory, 0, size, 0, &memoryPtr);
		memcpy(memoryPtr, data, size);
		vkUnmapMemory(myDevice->getDevice(), myBufferMemory);
	}

	void Buffer::updateBufferStage(void* data, size_t size) {
		auto stageBuffer = Buffer::create(myDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
		stageBuffer->updateBufferByMap(data, size);

		copyBuffer(stageBuffer->getBuffer(), myBuffer, static_cast<VkDeviceSize>(size));
	}

	void Buffer::copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size) {
		auto commandPool = CommandPool::create(myDevice);
		auto commandBuffer = CommandBuffer::create(myDevice, commandPool);

		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferCopy copyInfo{};
		copyInfo.size = size;

		commandBuffer->copyBuffer(srcBuffer, dstBuffer, 1, { copyInfo });

		commandBuffer->end();

		commandBuffer->submitSync(myDevice->getGraphicQueue(), VK_NULL_HANDLE);

	}
}