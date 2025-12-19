#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {

	class Sampler {
	public:
		using Ptr = std::shared_ptr<Sampler>;
		static Ptr create(const Device::Ptr& device) { return std::make_shared<Sampler>(device); }

		Sampler(const Device::Ptr &device);

		~Sampler();

		[[nodiscard]] auto getSampler() const { return mySampler; }

	private:
		Device::Ptr myDevice{ nullptr };
		VkSampler mySampler{ VK_NULL_HANDLE };
	};
}