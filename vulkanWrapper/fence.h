#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {

	//fence�ǿ���һ�ζ����ύ�ı�־����ͬ��semaphore�ǿ��Ƶ�һ�����ύ��Ϣ�ڲ�ͬ�׶μ��������ϵ
	//fence����һ�����У���GraphicQueue������һ�����ύ������ָ���Ƿ�ִ�����
	//fence���Խ���API����Ŀ��ƶ�semaphore����

	class Fence {
	public:
		using Ptr = std::shared_ptr<Fence>;

		static Ptr create(const Device::Ptr& device, bool signaled = true) {
			return std::make_shared<Fence>(device, signaled);
		}

		Fence(const Device::Ptr& device, bool signaled = true);	//signaled��ʾ����ʱ�Ƿ񼤷�

		~Fence();

		//��fence��Ϊ�Ǽ���̬
		void resetFence();	

		//���fenceû�б����������������ڴ˴��ȴ�����
		void block(uint64_t timeout = UINT64_MAX);

		[[nodiscard]] auto getFence() {
			return myFence;
		}

	private:
		VkFence myFence{ VK_NULL_HANDLE };
		Device::Ptr myDevice{ nullptr };
	};
}