#pragma once

#include "../base.h"

namespace FF::Wrapper {

	//Instance包含了Application相关信息，Layer层相关信息（其create和destroy函数需要取出），拓展相关信息

	class Instance 
	{
	public:
		using Ptr = std::shared_ptr<Instance>;

		static Ptr create(bool enableValidationLayer) {
			return std::make_shared<Instance>(enableValidationLayer);
		}

		Instance(bool enableValidationLayer);
		~Instance();

		//返回有多少种扩展
		void printAvailableExtensions();

		//返回我们想要开启的扩展
		std::vector<const char*> getRequiredExtensions();

		//检测是否支持validationLayer
		bool checkValidationLayerSupport();

		//设置myDebugger
		void setupDebugger();

		[[nodiscard]]VkInstance getInstance() const {		//nodiscard表示调用该函数后必须对返回值进行操作
			return myInstance;
		}

		[[nodiscard]] bool getEnableValidationLayer() const {	//获取是否使用检测层
			return myEnableValidationLayer;
		}

	private:
		VkInstance myInstance{ VK_NULL_HANDLE };
		bool myEnableValidationLayer{ false };
		VkDebugUtilsMessengerEXT myDebugger{ VK_NULL_HANDLE };
	};
}