#include "semaphore.h"

namespace FF::Wrapper {
	semaphore::semaphore(const Device::Ptr& device) {
		myDevice = device;

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(myDevice->getDevice(), &createInfo, nullptr, &mysemaphore) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create semaphore");
		}
	}

	semaphore::~semaphore() {
		if (mysemaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(myDevice->getDevice(), mysemaphore, nullptr);
		}
	}
}