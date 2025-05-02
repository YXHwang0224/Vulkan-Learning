#pragma once

#include "../base.h"
#include "device.h"
#include "description.h"

namespace FF::Wrapper {
	class DescriptorPool {
	public:
		using Ptr = std::shared_ptr<DescriptorPool>;
		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<DescriptorPool>(device);
		}

		DescriptorPool(const Device::Ptr& device);

		~DescriptorPool();

		void build(std::vector<UniformParameter::Ptr>& parameters, const int &frameCount);

		[[nodiscard]] auto getPool() {
			return myDescriptorPool;
		}

	private:
		//��Ҫ֪��ÿһ��Uniform���ж��ٸ�����Ϊ��Ԥ������ռ�
		//�ռ䲢����UniformBuffer�Ĵ�С�����Ǹ���ÿ��Uniform��ͬ���������Ͳ�ͬ������˵�ռ���ָ�������Ĵ�С���̺���ϵͳ�ڲ�
		VkDescriptorPool myDescriptorPool;
		Device::Ptr myDevice{ nullptr };
	};
}