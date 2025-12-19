#include "descriptorPool.h"

namespace FF::Wrapper {
	DescriptorPool::DescriptorPool(const Device::Ptr& device) {
		myDevice = device;
	}

	DescriptorPool::~DescriptorPool() {
		if (myDescriptorPool != VK_NULL_HANDLE){
			vkDestroyDescriptorPool(myDevice->getDevice(), myDescriptorPool, nullptr);
		}
	}

	void DescriptorPool::build(std::vector<UniformParameter::Ptr>& parameters, const int& frameCount) {

		//由于帧之间可能共用uniform，如果只有一个descriptorSet
		//那么我们对一个uniformbuffer修改时，可能会对之前提交给GPU的uniform进行更改
		
		//uniform缓冲有多少
		int uniformBufferCount = 0;
		int textureCount = 0;
		for (const auto& parameter : parameters) {
			if (parameter->myDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				uniformBufferCount++;
			}
			if (parameter->myDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				textureCount++;
			}
		}
		//纹理种类的uniform有多少

		//描述每一种uniform都有多少
		std::vector<VkDescriptorPoolSize> poolSize{};
		
		VkDescriptorPoolSize uniformBufferSize{};
		uniformBufferSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferSize.descriptorCount = uniformBufferCount * frameCount;
		poolSize.push_back(uniformBufferSize);

		VkDescriptorPoolSize textureSize{};
		textureSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureSize.descriptorCount = textureCount * frameCount; 	
		poolSize.push_back(textureSize);

		//创建Pool
		VkDescriptorPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		createInfo.pPoolSizes = poolSize.data();
		createInfo.maxSets = static_cast<uint32_t>(frameCount);

		if (vkCreateDescriptorPool(myDevice->getDevice(), &createInfo, nullptr, &myDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create Descriptor Pool");
		}
	}
}