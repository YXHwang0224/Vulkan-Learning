#pragma once

#include "../base.h"
#include "device.h"
#include "description.h"
#include "descriptorSetLayout.h"
#include "descriptorPool.h"

namespace FF::Wrapper {
	//ÿһ��ģ�͵���Ⱦ����Ҫһ��descriptorset���󶨵�λ����CommandBuffer
	class DescriptorSet {
	public:
		using Ptr = std::shared_ptr<DescriptorSet>;
		static Ptr create(
			const Device::Ptr& device,
			const std::vector<UniformParameter::Ptr> parameters,
			const DescriptorSetLayout::Ptr& layout,
			const DescriptorPool::Ptr& pool,
			int frameCount
		) {
			return std::make_shared<DescriptorSet>(device, parameters, layout, pool, frameCount);
		}

		DescriptorSet(
			const Device::Ptr& device,
			const std::vector<UniformParameter::Ptr> parameters,
			const DescriptorSetLayout::Ptr& layout,
			const DescriptorPool::Ptr& pool,
			int frameCount
		);

		~DescriptorSet();

		[[nodiscard]] auto getDescriptorSet(int frameCount) {
			return myDescriptorSets[frameCount];
		}


	private:
		std::vector<VkDescriptorSet> myDescriptorSets{};
		Device::Ptr myDevice{ nullptr };
	};
}