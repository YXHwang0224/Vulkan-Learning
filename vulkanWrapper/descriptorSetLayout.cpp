#include "descriptorSetLayout.h"

namespace FF::Wrapper {

	DescriptorSetLayout::DescriptorSetLayout(const Device::Ptr& device) {
		myDevice = device;
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		if (myLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(myDevice->getDevice(), myLayout, nullptr);
		}
	}

	void DescriptorSetLayout::build(const std::vector<UniformParameter::Ptr>& parameters){
		myParameters = parameters;

		if (myLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(myDevice->getDevice(), myLayout, nullptr);
		}

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

		for (const auto& parameter : myParameters) {
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.descriptorType = parameter->myDescriptorType;
			layoutBinding.binding = parameter->myBinding;
			layoutBinding.stageFlags = parameter->myStage;
			layoutBinding.descriptorCount = parameter->myCount;

			layoutBindings.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		createInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(myDevice->getDevice(), &createInfo, nullptr, &myLayout) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create desciptor set layout");
		}
	}
}