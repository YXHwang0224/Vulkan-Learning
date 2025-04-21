#pragma once

#include "../base.h"
#include "instance.h"
#include "windowSurface.h"

namespace FF::Wrapper {
	
	class Device {
	public:
		using Ptr = std::shared_ptr<Device>;

		static Ptr create(Instance::Ptr instance, WindowSurface::Ptr surface) {
			return std::make_shared<Device>(instance, surface);
		}
		
		Device(Instance::Ptr instance, WindowSurface::Ptr surface);

		~Device();

		void pickPhysicalDevice();		//ѡ���Կ�

		int rateDevice(VkPhysicalDevice device);	//���Կ����

		bool isDeviceSuitable(VkPhysicalDevice device);		//����Կ��Ƿ�������ǵ�Ҫ��

		void initQueueFamilies(VkPhysicalDevice device);	//Ѱ��QueueFamilies

		void createLogicalDevice();

		//�������ж����������Ƿ�ȫ���ҵ����ϵĶ�����ĸ�������
		bool isQueueFamilyComplete();

		[[nodiscard]] VkDevice getDevice() {
			return myDecive;
		}

		[[nodiscard]] VkPhysicalDevice getPhysicalDevice() {
			return myPhysicalDevice;
		}

		[[nodiscard]] std::optional<uint32_t> getGraphicQueueFamily() {
			return myGraphicQueueFamily;
		}

		[[nodiscard]] std::optional<uint32_t> getPresentQueueFamily() {
			return myPresentQueueFamily;
		}

	private:
		VkPhysicalDevice myPhysicalDevice{ VK_NULL_HANDLE };
		Instance::Ptr myInstance{ nullptr };
		WindowSurface::Ptr mySurface{ nullptr };

		//�洢��ǰ��Ⱦ����������id
		std::optional<uint32_t> myGraphicQueueFamily;
		VkQueue myGraphicQueue{ VK_NULL_HANDLE };

		//��ʾ����������ʾ����
		std::optional<uint32_t> myPresentQueueFamily;
		VkQueue myPresentQueue{ VK_NULL_HANDLE };

		//�߼��豸
		VkDevice myDecive{ VK_NULL_HANDLE };
	};
}