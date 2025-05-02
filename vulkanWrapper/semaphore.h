#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {

	//�ź�������ֻ��Ҫָ��������

	class semaphore {
	public:
		using Ptr = std::shared_ptr<semaphore>;
		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<semaphore>(device);
		}

		semaphore(const Device::Ptr& device);

		~semaphore();

		[[nodiscard]] auto getsemaphore() {
			return mysemaphore;
		}

	private:
		VkSemaphore mysemaphore{ VK_NULL_HANDLE };
		Device::Ptr myDevice{ nullptr };
	};
}