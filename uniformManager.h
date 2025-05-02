#pragma once

#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/description.h"
#include "vulkanWrapper/descriptorSet.h"
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

		void init(const Wrapper::Device::Ptr& device, int frameCount);

		void update(const VPMatrices& vpMatrix, const ObjectUniform& objectUniform, const int& frameCount);

		[[nodiscard]] auto getDescriptorLayout() {
			return myDescriptorSetLayout;
		}

		[[nodiscard]] auto getDescriptorSet(int frameCount) {
			return myDescriptorSet->getDescriptorSet(frameCount);
		}



	private:
		std::vector<Wrapper::UniformParameter::Ptr> myUniformParameters;

		Wrapper::DescriptorSetLayout::Ptr myDescriptorSetLayout{ nullptr };
		Wrapper::DescriptorPool::Ptr myDescriptorPool{ nullptr };
		Wrapper::DescriptorSet::Ptr myDescriptorSet{ nullptr };
	};
}