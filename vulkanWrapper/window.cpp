#include "window.h"

namespace FF::Wrapper {
	Window::Window(const int& width, const int& height) {
		myHeight = height;
		myWidth = width;
		glfwInit();

		//���û������ر�OpenGL�Ľӿ�
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		//���û�������ֹ�������ڴ�С
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		myWindow = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
		if (!myWindow) {
			std::cerr << "Error: failed to create window" << std::endl;
		}
	}

	Window::~Window() {
		glfwDestroyWindow(myWindow);
		glfwTerminate();			//�˳�
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(myWindow);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}
}