#include "device.h"

namespace FF::Wrapper {

	//����device����Ҫ����չ
	const std::vector<const char*> deciceRequiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME		//��������������Ҫ����չ
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

		std::multimap<int, VkPhysicalDevice> candidates;	//�Կ�������ͼ
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

		VkPhysicalDeviceProperties deviceProp;		//�Կ�����
		vkGetPhysicalDeviceProperties(device, &deviceProp);		//��ȡ�Կ����ԣ��豸���ƣ����ͣ�֧��Vulkan�İ汾)

		VkPhysicalDeviceFeatures deviceFeatures;	//�Կ�����
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);	//��ȡ�Կ����ԣ�����ѹ�����������������ԣ����ӿ���Ⱦ)

		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		score += deviceProp.limits.maxImageDimension2D;		//���֧�ֶ�������
		
		if (!deviceFeatures.geometryShader) {				//�ж��Ƿ�֧�ּ�����ɫ��
			return 0;
		}

		return score;
	}

	bool Device::isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProp;		//�Կ�����
		vkGetPhysicalDeviceProperties(device, &deviceProp);		//��ȡ�Կ����ԣ��豸���ƣ����ͣ�֧��Vulkan�İ汾

		VkPhysicalDeviceFeatures deviceFeatures;	//�Կ�����
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);	//��ȡ�Կ����ԣ�����ѹ�����������������ԣ����ӿ���Ⱦ

		return deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
	}

	void Device::initQueueFamilies(VkPhysicalDevice device) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		
		int i = 0;	//���������������
		for (const auto& queueFamily : queueFamilies) {
			//���������ܲ������е���������0�Ҷ������������Ⱦ������У���λ����ʵ�֣�
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				myGraphicQueueFamily = i;
			}

			//���������ܲ������е���������0�Ҷ������������ʾ������У�ʹ��GetSurface����ʵ�֣�
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mySurface->getSurface(), &presentSupport);

			if (presentSupport) {
				myPresentQueueFamily = i;
			}

			if (isQueueFamilyComplete()) {	//����ж�Ӧ���������˳�
				break;
			}

			++i;
		}
	}

	void Device::createLogicalDevice() {
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		std::set<uint32_t> queueFamilies = { myGraphicQueueFamily.value(), myPresentQueueFamily.value() };

		float queuePriority = 1.0;			//����ֵ

		for (uint32_t queueFamily : queueFamilies) {
			//��д������Ϣ
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = myGraphicQueueFamily.value();	//�����������
			queueCreateInfo.queueCount = 1;		//������������
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		//��д�߼��豸������Ϣ
		VkDeviceCreateInfo deviceCreatInfo = {};

		VkPhysicalDeviceFeatures deviceFeatures = {};
		
		deviceCreatInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreatInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreatInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreatInfo.enabledExtensionCount = static_cast<uint32_t>(deciceRequiredExtensions.size());
		deviceCreatInfo.ppEnabledExtensionNames = deciceRequiredExtensions.data();
		deviceCreatInfo.pEnabledFeatures = &deviceFeatures;

		//layer��
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