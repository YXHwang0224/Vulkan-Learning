#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {

	//CommandPool依据Device中的渲染队列和指令池内存属性建立

	class CommandPool {
	public:
		using Ptr = std::shared_ptr<CommandPool>;
		static Ptr create(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
			return std::make_shared<CommandPool>(device, flag);
		}

		CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		~CommandPool();

		[[nodiscard]] auto getCommandPool() {
			return myCommandPool;
		}

	private:
		VkCommandPool myCommandPool{ VK_NULL_HANDLE };
		Device::Ptr mydevice{ nullptr };
	};

}