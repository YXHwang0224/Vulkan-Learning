#include "semaphore.h"

namespace FF::Wrapper {
	Semaphore::Semaphore(const Device::Ptr& device) {
		myDevice = device;

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(myDevice->getDevice(), &createInfo, nullptr, &mySemaphore) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create semaphore");
		}
	}

	Semaphore::~Semaphore() {
		if (mySemaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(myDevice->getDevice(), mySemaphore, nullptr);
		}
	}
}