#pragma once

#include "../base.h"
#include "buffer.h"

namespace FF::Wrapper {

	//��Ҫ֪���������е�������Щuniform��ÿһ��������binding��ÿһ����ʲô����
	struct UniformParameter {
		using Ptr = std::shared_ptr<UniformParameter>;
		static Ptr create() {
			return std::make_shared<UniformParameter>();
		}

		size_t mySize{ 0 };
		uint32_t myBinding{ 0 };

		//����һ��binding�㣬���ܴ�����uniform�����count�ʹ�������Ĵ�С
		//����������Ϊ��λ������Ҫʹ��IndexDescriptor����
		uint32_t myCount{ 0 };
		VkDescriptorType myDescriptorType;
		VkShaderStageFlagBits myStage;

		std::vector<Buffer::Ptr> myBuffers{};	//��ʾ���uniform����Щbuffer�д���
	};
}