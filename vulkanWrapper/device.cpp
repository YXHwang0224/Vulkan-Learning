#include "device.h"

namespace FF::Wrapper {

	//构建device所需要的扩展
	const std::vector<const char*> deciceRequiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME		//构建交换链所需要的扩展
	};

	Device::Device(Instance::Ptr instance, WindowSurface::Ptr surface) {
		myInstance = instance;
		mySurface = surface;

		pickPhysicalDevice();
		initQueueFamilies(myPhysicalDevice);
		createLogicalDevice();
	}

	Device::~Device() {
		vkDestroyDevice(myDecive, nullptr);
		mySurface.reset();
		myInstance.reset();
	}

	void Device::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(myInstance->getInstance(), &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("Error:failed to enumerate physical device");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(myInstance->getInstance(), &deviceCount, devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;	//显卡的排序图
		for (const auto& device : devices) {
			int score = rateDevice(device);
			candidates.insert(std::make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second)) {
			myPhysicalDevice = candidates.rbegin()->second;
		}

		if (myPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Error:failed to enumerate physical device");
		}
	}

	int Device::rateDevice(VkPhysicalDevice device) {
		int score = 0;

		VkPhysicalDeviceProperties deviceProp;		//显卡属性
		vkGetPhysicalDeviceProperties(device, &deviceProp);		//获取显卡属性（设备名称，类型，支持Vulkan的版本)

		VkPhysicalDeviceFeatures deviceFeatures;	//显卡特性
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);	//获取显卡特性（纹理压缩，浮点数运算特性，多视口渲染)

		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		score += deviceProp.limits.maxImageDimension2D;		//最大支持多大的纹理
		
		if (!deviceFeatures.geometryShader) {				//判断是否支持几何着色器
			return 0;
		}

		return score;
	}

	bool Device::isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProp;		//显卡属性
		vkGetPhysicalDeviceProperties(device, &deviceProp);		//获取显卡属性（设备名称，类型，支持Vulkan的版本

		VkPhysicalDeviceFeatures deviceFeatures;	//显卡特性
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);	//获取显卡特性（纹理压缩，浮点数运算特性，多视口渲染

		return deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
	}

	void Device::initQueueFamilies(VkPhysicalDevice device) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		
		int i = 0;	//计数队列族的索引
		for (const auto& queueFamily : queueFamilies) {
			//检查队列族能产生队列的能力大于0且队列族包含有渲染任务队列（用位运算实现）
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				myGraphicQueueFamily = i;
			}

			//检查队列族能产生队列的能力大于0且队列族包含有显示任务队列（使用GetSurface函数实现）
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mySurface->getSurface(), &presentSupport);

			if (presentSupport) {
				myPresentQueueFamily = i;
			}

			if (isQueueFamilyComplete()) {	//如果有对应索引，就退出
				break;
			}

			++i;
		}
	}

	void Device::createLogicalDevice() {
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		std::set<uint32_t> queueFamilies = { myGraphicQueueFamily.value(), myPresentQueueFamily.value() };

		float queuePriority = 1.0;			//优先值

		for (uint32_t queueFamily : queueFamilies) {
			//填写创建信息
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = myGraphicQueueFamily.value();	//队列族的索引
			queueCreateInfo.queueCount = 1;		//创建几个队列
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		//填写逻辑设备创建信息
		VkDeviceCreateInfo deviceCreatInfo = {};

		VkPhysicalDeviceFeatures deviceFeatures = {};
		
		deviceCreatInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreatInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreatInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreatInfo.enabledExtensionCount = static_cast<uint32_t>(deciceRequiredExtensions.size());
		deviceCreatInfo.ppEnabledExtensionNames = deciceRequiredExtensions.data();
		deviceCreatInfo.pEnabledFeatures = &deviceFeatures;

		//layer层
		if (myInstance->getEnableValidationLayer()) {
			deviceCreatInfo.enabledLayerCount = static_cast<uint32_t>(validationLayer.size());
			deviceCreatInfo.ppEnabledLayerNames = validationLayer.data();
		}
		else {
			deviceCreatInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(myPhysicalDevice, &deviceCreatInfo, nullptr, &myDecive) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create logical device");
		}

		vkGetDeviceQueue(myDecive, myGraphicQueueFamily.value(), 0, &myGraphicQueue);
		vkGetDeviceQueue(myDecive, myPresentQueueFamily.value(), 0, &myPresentQueue);
	}

	bool Device::isQueueFamilyComplete() {
		return myGraphicQueueFamily.has_value() && myPresentQueueFamily.has_value();
	}
}