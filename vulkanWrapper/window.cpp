#include "window.h"

namespace FF::Wrapper {
	Window::Window(const int& width, const int& height) {
		myHeight = height;
		myWidth = width;
		glfwInit();

		//设置环境，关闭OpenGL的接口
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		//设置环境，禁止调整窗口大小
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		myWindow = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
		if (!myWindow) {
			std::cerr << "Error: failed to create window" << std::endl;
		}
	}

	Window::~Window() {
		glfwDestroyWindow(myWindow);
		glfwTerminate();			//退出
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(myWindow);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}
}