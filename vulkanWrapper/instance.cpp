#include "instance.h"

namespace FF::Wrapper {
	//validationLayer�Ļص���������һ������Ϊ���صȼ���error��warning...�����ڶ�������Ϊ��Ϣ���ࣨӰ��Ч��...)������������Ϊ������Ϣ
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT massageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pMassageData,
		void* pUserData) {
		std::cout << "Validation Layer��" << pMassageData->pMessage << std::endl;

		return VK_FALSE;		//��ʾ�ص�������Ҫ��ֹ��������
	}

	//������������������debugMessenger����
	//��һ������Ϊһ��Instance���ڶ�������Ϊ�����ο���Info������������Ϊ�ڴ���أ����һ���Ǵ�����debugMessenger
	static VkResult CreatDubugUtilsMessagerEXT(VkInstance instance,
		VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* debugMessenger) {
		//���ڴ���������δ���룬����ʹ����������õ�������������ת������
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr) {
			func(instance, pCreateInfo, pAllocator, debugMessenger);
			return VK_SUCCESS;
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;		//������չû�б�Ӧ��
		}
	}

	//�����������������ٵ������������Ĵ���debugMessenger�ĺ���
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr) {
			return func(instance, debugMessenger, pAllocator);
		}

	}

	Instance::Instance(bool enableValidationLayer) {
		//Vulkan���������κ�handle�����������Ҫ�ṩһ��Info������ʹ��һ��Create��������

		myEnableValidationLayer = enableValidationLayer;		//ȷ���Ƿ�����֤��

		if (myEnableValidationLayer && !checkValidationLayerSupport()) {
			throw std::runtime_error("Error:validation layer is not supported");
		}

		VkApplicationInfo appInfo = {};							//VkApplicationInfo�д���Ԫ����
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;		//����appInfo�����ͣ���������һ��application����Ϣ
		appInfo.pApplicationName = "Vulkan";					//����
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);	//ʹ��VK_MAKE_VERSION������appInfo�İ汾��������
		appInfo.pEngineName = "No Engine";						//���������˴������ǣ�
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);		//����汾
		appInfo.apiVersion = VK_API_VERSION_1_0;					//Vulkan API�İ汾

		VkInstanceCreateInfo instCreateInfo = {};				//����Instanceʱʹ�õ���Ϣ��
		instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instCreateInfo.pApplicationInfo = &appInfo;				//��VkApplicationInfo��ָ�룩�󶨵�instCreateInfo

		//��չ���
		std::vector<const char*> extensions = getRequiredExtensions();		
		instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instCreateInfo.ppEnabledExtensionNames = extensions.data();

		//��������
		if (myEnableValidationLayer) {
			instCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayer.size());
			instCreateInfo.ppEnabledLayerNames = validationLayer.data();
		}
		else
			instCreateInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&instCreateInfo, nullptr, &myInstance) != VK_SUCCESS) {		//ʹ��instCreatInfo����Instance��myInstance�в���飬�ڶ�������Ϊ�ڴ����
			throw std::runtime_error("Error:failed to create instance");
		}

		setupDebugger();
		printAvailableExtensions();
	}

	Instance::~Instance() {
		DestroyDebugUtilsMessengerEXT(myInstance, myDebugger, nullptr);
		vkDestroyInstance(myInstance, nullptr);					//����Instance���ڶ�������Ϊ�ڴ����
	}

	void Instance::printAvailableExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);		//��ȡ��չ���������ڶ�����������������չ���飨������������
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());	//����չ����extensions��

		std::cout << "Available extensions:" << std::endl;

		for (const auto& extention : extensions) {
			std::cout << extention.extensionName << std::endl;
		}
	}

	std::vector<const char*> Instance::getRequiredExtensions() {
		uint32_t glfwExtensionsCount = 0;

		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);	//����Instance����Ҫ����չ�����������ṩ������

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);			//����validationLayer

		return extensions;
	}

	bool Instance::checkValidationLayerSupport() {
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);		//��ȡlayer������
		std::vector<VkLayerProperties> availablelayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availablelayers.data());

		for (const auto& layerName : validationLayer) {
			bool layerFound = false;

			for (const auto& layerProp : availablelayers) {
				if (std::strcmp(layerProp.layerName, layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}
		return true;
	}

	void Instance::setupDebugger() {
		if (!myEnableValidationLayer)
			return;

		VkDebugUtilsMessengerCreateInfoEXT creatInfo = {};
		creatInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		creatInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		creatInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		creatInfo.pfnUserCallback = debugCallBack;
		creatInfo.pUserData = nullptr;

		if (CreatDubugUtilsMessagerEXT(myInstance, &creatInfo, nullptr, &myDebugger)!=VK_SUCCESS) {
			throw std::runtime_error("ERROR:failed to create debugger");
		}
	}
}