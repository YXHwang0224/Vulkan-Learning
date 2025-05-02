#include "instance.h"

namespace FF::Wrapper {
	//validationLayer的回调函数，第一个参数为严重等级（error，warning...），第二个参数为信息分类（影响效率...)，第三个参数为返回信息
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT massageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pMassageData,
		void* pUserData) {
		std::cout << "Validation Layer；" << pMassageData->pMessage << std::endl;

		return VK_FALSE;		//表示回调函数不要中止函数调用
	}

	//辅助函数，辅助帮助debugMessenger建立
	//第一个参数为一个Instance，第二个参数为创建参考的Info，第三个参数为内存相关，最后一个是创建的debugMessenger
	static VkResult CreatDubugUtilsMessagerEXT(VkInstance instance,
		VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* debugMessenger) {
		//由于创建函数还未引入，所以使用这个函数拿到创建函数，并转换类型
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr) {
			func(instance, pCreateInfo, pAllocator, debugMessenger);
			return VK_SUCCESS;
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;		//返回扩展没有被应用
		}
	}

	//辅助函数，负责销毁掉上面引用来的创建debugMessenger的函数
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr) {
			return func(instance, debugMessenger, pAllocator);
		}

	}

	Instance::Instance(bool enableValidationLayer) {
		//Vulkan里面生成任何handle（句柄）都需要提供一个Info，而后使用一种Create函数生成

		myEnableValidationLayer = enableValidationLayer;		//确认是否开启验证层

		if (myEnableValidationLayer && !checkValidationLayerSupport()) {
			throw std::runtime_error("Error:validation layer is not supported");
		}

		VkApplicationInfo appInfo = {};							//VkApplicationInfo中储存元数据
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;		//设置appInfo的类型，表面其是一个application的信息
		appInfo.pApplicationName = "Vulkan";					//命名
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);	//使用VK_MAKE_VERSION函数对appInfo的版本进行设置
		appInfo.pEngineName = "No Engine";						//引擎名（此处不考虑）
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);		//引擎版本
		appInfo.apiVersion = VK_API_VERSION_1_0;					//Vulkan API的版本

		VkInstanceCreateInfo instCreateInfo = {};				//创建Instance时使用的信息体
		instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instCreateInfo.pApplicationInfo = &appInfo;				//将VkApplicationInfo（指针）绑定到instCreateInfo

		//扩展相关
		std::vector<const char*> extensions = getRequiredExtensions();		
		instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instCreateInfo.ppEnabledExtensionNames = extensions.data();

		//检验层相关
		if (myEnableValidationLayer) {
			instCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayer.size());
			instCreateInfo.ppEnabledLayerNames = validationLayer.data();
		}
		else
			instCreateInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&instCreateInfo, nullptr, &myInstance) != VK_SUCCESS) {		//使用instCreatInfo建立Instance至myInstance中并检查，第二个参数为内存分配
			throw std::runtime_error("Error:failed to create instance");
		}

		setupDebugger();
		printAvailableExtensions();
	}

	Instance::~Instance() {
		DestroyDebugUtilsMessengerEXT(myInstance, myDebugger, nullptr);
		vkDestroyInstance(myInstance, nullptr);					//销毁Instance，第二个参数为内存分配
	}

	void Instance::printAvailableExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);		//获取扩展的数量（第二个参数）与具体的扩展数组（第三个参数）
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());	//将扩展放入extensions中

		std::cout << "Available extensions:" << std::endl;

		for (const auto& extention : extensions) {
			std::cout << extention.extensionName << std::endl;
		}
	}

	std::vector<const char*> Instance::getRequiredExtensions() {
		uint32_t glfwExtensionsCount = 0;

		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);	//返回Instance中需要的扩展，并将数量提供给参数

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);			//开启validationLayer

		return extensions;
	}

	bool Instance::checkValidationLayerSupport() {
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);		//获取layer的数量
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