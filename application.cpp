#include "application.h"

namespace FF {

	void Application::Run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::initWindow() {
		myWindow = Wrapper::Window::create(WIDTH, HEIGHT);
	}

	void Application::initVulkan() {
		myInstance = Wrapper::Instance::creat(true);
		mySurface = Wrapper::WindowSurface::create(myInstance, myWindow);
		myDevice = Wrapper::Device::create(myInstance, mySurface);
		mySwapChain = Wrapper::SwapChain::create(myDevice, myWindow, mySurface);
	}

	void Application::mainLoop() {
		while (!myWindow->shouldClose()) {
			myWindow->pollEvents();		//分发鼠标键盘等信息
		}
	}

	void Application::cleanUp() {
		mySwapChain.reset();
		myDevice.reset();
		mySurface.reset();
		myInstance.reset();
		myWindow->~Window();
	}
}