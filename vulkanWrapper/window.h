#pragma once

#include "../base.h"

namespace FF {
	class Application;
}

namespace FF::Wrapper {

	class Window {
	public:
		using Ptr = std::shared_ptr<Window>;
		static Ptr create(const int& width, const int& height) {
			return std::make_shared<Window>(width, height);
		}

		Window(const int& width, const int& height);

		~Window();

		bool shouldClose();		//检测是否需要关闭窗口

		void pollEvents();			//检测事件

		[[nodiscard]] GLFWwindow* getWindow() const {
			return myWindow;
		}

		void setApplication(const std::shared_ptr<Application>& application) {
			myApplication = application;
		}

		void processEvents();		//处理事件

	public:
		bool myWindowResized{ false };
		std::weak_ptr<Application> myApplication;

	private:
		
		int myWidth{ 0 };
		int myHeight{ 0 };
		GLFWwindow* myWindow{ nullptr };
		
	};
}
