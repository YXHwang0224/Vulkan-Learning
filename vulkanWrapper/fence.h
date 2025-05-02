#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {

	//fence是控制一次队列提交的标志，不同于semaphore是控制单一命令提交信息内不同阶段间的依赖关系
	//fence控制一个队列（如GraphicQueue）里面一次性提交的所有指令是否执行完毕
	//fence可以进行API级别的控制而semaphore不行

	class Fence {
	public:
		using Ptr = std::shared_ptr<Fence>;

		static Ptr create(const Device::Ptr& device, bool signaled = true) {
			return std::make_shared<Fence>(device, signaled);
		}

		Fence(const Device::Ptr& device, bool signaled = true);	//signaled表示构建时是否激发

		~Fence();

		//将fence置为非激发态
		void resetFence();	

		//如果fence没有被激发，将其阻塞在此处等待激发
		void block(uint64_t timeout = UINT64_MAX);

		[[nodiscard]] auto getFence() {
			return myFence;
		}

	private:
		VkFence myFence{ VK_NULL_HANDLE };
		Device::Ptr myDevice{ nullptr };
	};
}