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

#include "uniformManager.h"
#include "model.h"

namespace FF {

	class Application		//�����
	{
	public:
		Application() = default;

		~Application() = default;

		void Run();

	private:
		void initWindow();

		void initVulkan();

		void mainLoop();

		void render();

		void cleanUp();

	private:
		void createPipeline();
		void createRenderPass();
		void createCommandBuffer();
		void createSemaphore();
		//�����ڴ�С�����任ʱ���ؽ�����������ص�Frame��View...����任
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

		std::vector<Wrapper::semaphore::Ptr> myImageAvaliablesemaphores;
		std::vector<Wrapper::semaphore::Ptr> myRenderFinishedsemaphores;
		std::vector<Wrapper::Fence::Ptr> myFences;

		UniformManager::Ptr myUniformManger{ nullptr };

		Model::Ptr myModel{ nullptr };

		VPMatrices myVPMatrices;
	};
}