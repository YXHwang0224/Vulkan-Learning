#pragma once

#include "../base.h"
#include "instance.h"
#include "window.h"

namespace FF::Wrapper {

	//使用Instance和window建立surface

	class WindowSurface {
	public:
		using Ptr = std::shared_ptr<WindowSurface>;
		static Ptr create(Instance::Ptr instance, Window::Ptr window) {
			return std::make_shared<WindowSurface>(instance, window);
		}

		WindowSurface(Instance::Ptr instance, Window::Ptr window);

		~WindowSurface();

		VkSurfaceKHR getSurface() {
			return mySurface;
		}

	private:
		VkSurfaceKHR mySurface{ VK_NULL_HANDLE };
		Instance::Ptr myInstance{ VK_NULL_HANDLE };
	};
}