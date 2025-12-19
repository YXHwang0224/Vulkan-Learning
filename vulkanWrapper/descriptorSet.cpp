#include "descriptorSet.h"

namespace FF::Wrapper {
	DescriptorSet::DescriptorSet(
		const Device::Ptr& device,
		const std::vector<UniformParameter::Ptr> parameters,
		const DescriptorSetLayout::Ptr& layout,
		const DescriptorPool::Ptr& pool,
		int frameCount
	) {
		myDevice = device;

		std::vector<VkDescriptorSetLayout> layouts(frameCount, layout->getLayout());

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool->getPool();
		allocInfo.descriptorSetCount = frameCount;
		allocInfo.pSetLayouts = layouts.data();

		myDescriptorSets.resize(frameCount);
		if (vkAllocateDescriptorSets(myDevice->getDevice(), &allocInfo, &myDescriptorSets[0]) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to allocate descriptor sets");
		}
		
		for (int i = 0; i < frameCount; ++i) {
			//对每个DescriptorSet，我们需要把每个parameters描述信息写入其中
			std::vector<VkWriteDescriptorSet> descriptorSetWrites{};
			for (const auto& parameter : parameters) {
				VkWriteDescriptorSet descriptorSetWrite{};
				descriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorSetWrite.dstSet = myDescriptorSets[i];
				descriptorSetWrite.dstArrayElement = 0;
				descriptorSetWrite.descriptorType = parameter->myDescriptorType;
				descriptorSetWrite.descriptorCount = parameter->myCount;
				descriptorSetWrite.dstBinding = parameter->myBinding;

				if (parameter->myDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					descriptorSetWrite.pBufferInfo = &parameter->myBuffers[i]->getBufferInfo();
				}

				if (parameter->myDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
					descriptorSetWrite.pImageInfo = &parameter->myTexture->getImageInfo();
				}

				descriptorSetWrites.push_back(descriptorSetWrite);
			}

			vkUpdateDescriptorSets(myDevice->getDevice(), static_cast<uint32_t>(descriptorSetWrites.size()), descriptorSetWrites.data(), 0, nullptr);
		}
	}

	DescriptorSet::~DescriptorSet() {

	}
}