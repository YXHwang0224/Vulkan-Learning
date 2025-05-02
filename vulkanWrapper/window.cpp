#include "window.h"

namespace FF::Wrapper {

	static void windowResized(GLFWwindow* window, int width, int height) {
		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pUserData->myWindowResized = true;
	}

	Window::Window(const int& width, const int& height) {
		myHeight = height;
		myWidth = width;
		glfwInit();

		//���û������ر�OpenGL�Ľӿ�
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		//���û�������ֹ�������ڴ�С
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		myWindow = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
		if (!myWindow) {
			std::cerr << "Error: failed to create window" << std::endl;
		}

		glfwSetWindowUserPointer(myWindow, this);
		glfwSetFramebufferSizeCallback(myWindow, windowResized);
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