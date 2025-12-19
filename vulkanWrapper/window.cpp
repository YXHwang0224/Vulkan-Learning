#include "window.h"
#include "../application.h"
#include "../Camera.h"

namespace FF::Wrapper {

	static void windowResized(GLFWwindow* window, int width, int height) {
		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pUserData->myWindowResized = true;
	}

	static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos) {
		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		auto app = pUserData->myApplication;
		if (!app.expired()) {
			auto appReal = app.lock();
			appReal->onMouseMove(xpos, ypos);
		}
	}

	Window::Window(const int& width, const int& height) {
		myWidth = width;
		myHeight = height;

		glfwInit();

		//设置环境，关掉opengl API
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		myWindow = glfwCreateWindow(myWidth, myHeight, "vulkan window", nullptr, nullptr);
		if (!myWindow) {
			std::cerr << "Error: failed to create window" << std::endl;
		}

		glfwSetWindowUserPointer(myWindow, this);
		glfwSetFramebufferSizeCallback(myWindow, windowResized);
		glfwSetCursorPosCallback(myWindow, cursorPosCallBack);

		glfwSetInputMode(myWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	Window::~Window() {
		glfwDestroyWindow(myWindow);
		glfwTerminate();
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(myWindow);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}

	void Window::processEvents() {
		if (myApplication.expired()) {
			return;
		}

		auto app = myApplication.lock();

		if (glfwGetKey(myWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			exit(0);
		}

		if (glfwGetKey(myWindow, GLFW_KEY_W) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_FRONT);
		}

		if (glfwGetKey(myWindow, GLFW_KEY_S) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_BACK);
		}

		if (glfwGetKey(myWindow, GLFW_KEY_A) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_LEFT);
		}

		if (glfwGetKey(myWindow, GLFW_KEY_D) == GLFW_PRESS) {
			app->onKeyDown(CAMERA_MOVE::MOVE_RIGHT);
		}
	}
}