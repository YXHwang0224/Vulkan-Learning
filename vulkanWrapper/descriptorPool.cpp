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

		//����֮֡����ܹ���uniform�����ֻ��һ��descriptorSet
		//��ô���Ƕ�һ��uniformbuffer�޸�ʱ�����ܻ��֮ǰ�ύ��GPU��uniform���и���
		
		//uniform�����ж���
		int uniformBufferCount = 0;
		for (const auto& parameter : parameters) {
			if (parameter->myDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				uniformBufferCount++;
			}
		}
		//���������uniform�ж���

		//����ÿһ��uniform���ж���
		std::vector<VkDescriptorPoolSize> poolSize{};
		
		VkDescriptorPoolSize uniformBufferSize{};
		uniformBufferSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferSize.descriptorCount = uniformBufferCount * frameCount;
		poolSize.push_back(uniformBufferSize);

		//����Pool
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