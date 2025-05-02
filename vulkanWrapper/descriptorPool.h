#pragma once

#include "../base.h"
#include "device.h"
#include "description.h"

namespace FF::Wrapper {
	class DescriptorPool {
	public:
		using Ptr = std::shared_ptr<DescriptorPool>;
		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<DescriptorPool>(device);
		}

		DescriptorPool(const Device::Ptr& device);

		~DescriptorPool();

		void build(std::vector<UniformParameter::Ptr>& parameters, const int &frameCount);

		[[nodiscard]] auto getPool() {
			return myDescriptorPool;
		}

	private:
		//需要知道每一种Uniform都有多少个才能为其预留分配空间
		//空间并不是UniformBuffer的大小，而是根据每种Uniform不同，描述符就不同，所以说空间是指描述符的大小，蕴含在系统内部
		VkDescriptorPool myDescriptorPool;
		Device::Ptr myDevice{ nullptr };
	};
}