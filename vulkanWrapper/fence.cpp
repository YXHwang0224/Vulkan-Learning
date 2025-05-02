#include "fence.h"

namespace FF::Wrapper {
	Fence::Fence(const Device::Ptr& device, bool signaled) {
		myDevice = device;

		VkFenceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		if (vkCreateFence(myDevice->getDevice(), &createInfo, nullptr, &myFence) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create fence");
		}
	}

	Fence::~Fence() {
		if (myFence != VK_NULL_HANDLE) {
			vkDestroyFence(myDevice->getDevice(), myFence, nullptr);
		}
	}
	
	void Fence::resetFence() {
		vkResetFences(myDevice->getDevice(), 1, &myFence);
	}

	
	void Fence::block(uint64_t timeout) {
		vkWaitForFences(myDevice->getDevice(), 1, &myFence, VK_TRUE, timeout);
	}
}