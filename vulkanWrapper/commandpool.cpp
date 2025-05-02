
#include "commandpool.h"

namespace FF::Wrapper {
	CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag) {
		mydevice = device;

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = mydevice->getGraphicQueueFamily().value();
		//ָ���޸����ԡ�ָ����ڴ�����
		//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT�����������CommandBuffer���Ե������£���������
		//VK_COMMAND_POOL_CREATE_TRANSINENT_BIT��ÿһ��CommandBuffer���ܵ���reset�����뼯�����ã�vkResetCommandPool��
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