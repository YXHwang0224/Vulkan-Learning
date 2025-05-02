#include "uniformManager.h"

namespace FF {
	UniformManager::UniformManager() {

	}

	UniformManager::~UniformManager() {

	}

	void UniformManager::init(const Wrapper::Device::Ptr& device, int frameCount) {
		auto VPParameter = Wrapper::UniformParameter::create();
		VPParameter->myBinding = 0;
		VPParameter->myCount = 1;
		VPParameter->myDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		VPParameter->mySize = sizeof(VPMatrices);
		VPParameter->myStage = VK_SHADER_STAGE_VERTEX_BIT;

		for (int i = 0; i < frameCount; ++i) {
			auto buffer = Wrapper::Buffer::createUniformBuffer(device, VPParameter->mySize, nullptr);
			VPParameter->myBuffers.push_back(buffer);
		}

		myUniformParameters.push_back(VPParameter);

		auto objectParameter = Wrapper::UniformParameter::create();
		objectParameter->myBinding = 1;
		objectParameter->myCount = 1;
		objectParameter->myDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectParameter->mySize = sizeof(ObjectUniform);
		objectParameter->myStage = VK_SHADER_STAGE_VERTEX_BIT;

		for (int i = 0; i < frameCount; ++i) {
			auto buffer = Wrapper::Buffer::createUniformBuffer(device, objectParameter->mySize, nullptr);
			objectParameter->myBuffers.push_back(buffer);
		}

		myUniformParameters.push_back(objectParameter);

		myDescriptorSetLayout = Wrapper::DescriptorSetLayout::create(device);
		myDescriptorSetLayout->build(myUniformParameters);

		myDescriptorPool = Wrapper::DescriptorPool::create(device);
		myDescriptorPool->build(myUniformParameters, frameCount);
		myDescriptorSet = Wrapper::DescriptorSet::create(device, myUniformParameters, myDescriptorSetLayout, myDescriptorPool, frameCount);
	}

	void UniformManager::update(const VPMatrices& vpMatrices, const ObjectUniform& objectUniform, const int& frameCount) {
		//更新VP矩阵
		myUniformParameters[0]->myBuffers[frameCount]->updateBufferByMap((void*)(&vpMatrices), sizeof(VPMatrices));

		//更新objectUniform
		myUniformParameters[1]->myBuffers[frameCount]->updateBufferByMap((void*)(&objectUniform), sizeof(objectUniform));
	}
}