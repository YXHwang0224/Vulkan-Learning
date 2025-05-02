
#include "commandpool.h"

namespace FF::Wrapper {
	CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag) {
		mydevice = device;

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = mydevice->getGraphicQueueFamily().value();
		//指令修改属性、指令池内存属性
		//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT：分配出来的CommandBuffer可以单独更新，单独重置
		//VK_COMMAND_POOL_CREATE_TRANSINENT_BIT：每一个CommandBuffer不能单独reset，必须集体重置（vkResetCommandPool）
		createInfo.flags = flag;

		if (vkCreateCommandPool(mydevice->getDevice(), &createInfo, nullptr, &myCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create command pool");
		}
	}

	CommandPool::~CommandPool() {
		if (myCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(mydevice->getDevice(), myCommandPool, nullptr);
		}
	}

}