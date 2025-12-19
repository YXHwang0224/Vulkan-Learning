#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {

	//信号量创建只需要指定其类型

	class Semaphore {
	public:
		using Ptr = std::shared_ptr<Semaphore>;
		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<Semaphore>(device);
		}

		Semaphore(const Device::Ptr& device);

		~Semaphore();

		[[nodiscard]] auto getSemaphore() {
			return mySemaphore;
		}

	private:
		VkSemaphore mySemaphore{ VK_NULL_HANDLE };
		Device::Ptr myDevice{ nullptr };
	};
}