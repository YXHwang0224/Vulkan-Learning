#pragma once

#include "../base.h"
#include "description.h"
#include "device.h"

namespace FF::Wrapper {

	class DescriptorSetLayout {
	public:
		using Ptr = std::shared_ptr<DescriptorSetLayout>;
		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<DescriptorSetLayout>(device);
		}

		DescriptorSetLayout(const Device::Ptr& device);

		~DescriptorSetLayout();

		void build(const std::vector<UniformParameter::Ptr>& parameters);

		[[nodiscard]] auto getLayout() {
			return myLayout;
		}

	private:
		VkDescriptorSetLayout myLayout{ VK_NULL_HANDLE };
		Device::Ptr myDevice{ nullptr };

		std::vector<UniformParameter::Ptr> myParameters;
	};
}
