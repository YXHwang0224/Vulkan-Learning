#include "windowSurface.h"

namespace FF::Wrapper {
	WindowSurface::WindowSurface(Instance::Ptr instance, Window::Ptr window) {
		myInstance = instance;
		if (glfwCreateWindowSurface(instance->getInstance(), window->getWindow(), nullptr, &mySurface) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create surface");
		}
	}

	WindowSurface::~WindowSurface() {
		vkDestroySurfaceKHR(myInstance->getInstance(), mySurface, nullptr);
		myInstance.reset();
	}
}