#pragma once

#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/description.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/commandpool.h"
#include "base.h"

namespace FF {
	class UniformManager {
	public:
		using Ptr = std::shared_ptr<UniformManager>;
		static Ptr create() {
			return std::make_shared<UniformManager>();
		}

		UniformManager();

		~UniformManager();

		void init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, int frameCount);

		void update(const VPMatrices& vpMatrix, const ObjectUniform& objectUniform, const int& frameCount);

		[[nodiscard]] auto getDescriptorLayout() {
			return myDescriptorSetLayout;
		}

		[[nodiscard]] auto getDescriptorSet(int frameCount) {
			return myDescriptorSet->getDescriptorSet(frameCount);
		}



	private:
		std::vector<Wrapper::UniformParameter::Ptr> myUniformParameters;

		Wrapper::Device::Ptr myDevice{ nullptr };
		Wrapper::CommandPool::Ptr myCommandPool{ nullptr };
		Wrapper::DescriptorSetLayout::Ptr myDescriptorSetLayout{ nullptr };
		Wrapper::DescriptorPool::Ptr myDescriptorPool{ nullptr };
		Wrapper::DescriptorSet::Ptr myDescriptorSet{ nullptr };
	};
}