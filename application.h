#pragma once

#include "base.h"
#include "vulkanWrapper/instance.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/window.h"
#include "vulkanWrapper/windowSurface.h"
#include "vulkanWrapper/swapChain.h"
#include "vulkanWrapper/shader.h"
#include "vulkanWrapper/pipeline.h"
#include "vulkanWrapper/renderpass.h"

namespace FF {

	const int HEIGHT = 1080;
	const int WIDTH = 1440;

	class Application		//Èë¿ÚÀà
	{
	public:
		Application() = default;

		~Application() = default;

		void Run();

	private:
		void initWindow();

		void initVulkan();

		void mainLoop();

		void cleanUp();

	private:
		void createPipeline();

		void createRenderPass();

	private:
		Wrapper::Window::Ptr myWindow{ nullptr };
		Wrapper::Instance::Ptr myInstance{ nullptr };
		Wrapper::Device::Ptr myDevice{ nullptr };
		Wrapper::WindowSurface::Ptr mySurface{ nullptr };
		Wrapper::SwapChain::Ptr mySwapChain{ nullptr };
		Wrapper::Pipeline::Ptr myPipeline{ nullptr };
		Wrapper::RenderPass::Ptr myRenderPass{ nullptr };
	};
}