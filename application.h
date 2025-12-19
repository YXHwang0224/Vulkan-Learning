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
#include "vulkanWrapper/commandpool.h"
#include "vulkanWrapper/commandbuffer.h"
#include "vulkanWrapper/semaphore.h"
#include "vulkanWrapper/fence.h"
#include "vulkanWrapper/image.h"
#include "vulkanWrapper/sampler.h"

#include "texture/texture.h"

#include "uniformManager.h"
#include "model.h"
#include "camera.h"

namespace FF {

	class Application:public std::enable_shared_from_this<Application>	//入口类
	{
	public:
		Application() = default;

		~Application() = default;

		void run();
		
		void onMouseMove(double x, double y);

		void onKeyDown(CAMERA_MOVE moveDirection);

	private:
		void initWindow();

		void initVulkan();

		void mainLoop();

		void render();

		void cleanUp();

	private:
		void createPipeline();
		void createRenderPass();
		void createCommandBuffers();
		void createSemaphore();
		
		//当窗口大小发生变换时，重建交换链，相关的Frame，View...都会变换
		void recreateSwapChain();
		void cleanupSwapChain();

	private:
		unsigned int myWidth = 1440;
		unsigned int myHeight = 1080;

	private:
		int myCurrentFrame{ 0 };

		Wrapper::Window::Ptr myWindow{ nullptr };
		Wrapper::Instance::Ptr myInstance{ nullptr };
		Wrapper::Device::Ptr myDevice{ nullptr };
		Wrapper::WindowSurface::Ptr mySurface{ nullptr };
		Wrapper::SwapChain::Ptr mySwapChain{ nullptr };
		Wrapper::Pipeline::Ptr myPipeline{ nullptr };
		Wrapper::RenderPass::Ptr myRenderPass{ nullptr };
		Wrapper::CommandPool::Ptr myCommandPool{ nullptr };

		std::vector<Wrapper::CommandBuffer::Ptr> myCommandBuffers{};

		std::vector<Wrapper::Semaphore::Ptr> myImageAvailableSemaphores;
		std::vector<Wrapper::Semaphore::Ptr> myRenderFinishedSemaphores;
		std::vector<Wrapper::Fence::Ptr> myFences;

		UniformManager::Ptr myUniformManager{ nullptr };

		Model::Ptr myModel{ nullptr };

		VPMatrices myVPMatrices;
		Camera myCamera;
	};
}