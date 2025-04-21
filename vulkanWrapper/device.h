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

		void pickPhysicalDevice();		//选择显卡

		int rateDevice(VkPhysicalDevice device);	//给显卡打分

		bool isDeviceSuitable(VkPhysicalDevice device);		//检查显卡是否符合我们的要求

		void initQueueFamilies(VkPhysicalDevice device);	//寻找QueueFamilies

		void createLogicalDevice();

		//返回所有队列族条件是否全部找到符合的队列族的辅助函数
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

		//存储当前渲染任务队列族的id
		std::optional<uint32_t> myGraphicQueueFamily;
		VkQueue myGraphicQueue{ VK_NULL_HANDLE };

		//显示队列族与显示队列
		std::optional<uint32_t> myPresentQueueFamily;
		VkQueue myPresentQueue{ VK_NULL_HANDLE };

		//逻辑设备
		VkDevice myDecive{ VK_NULL_HANDLE };
	};
}