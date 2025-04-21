#pragma once

#include "../base.h"
#include "device.h"
#include "window.h"
#include "windowSurface.h"
#include "renderpass.h"

namespace FF::Wrapper {

	struct SwapChainSupportInfo							//交换链相关的信息
	{
		VkSurfaceCapabilitiesKHR myCapabilities;		//交换链的基础特性
		std::vector<VkSurfaceFormatKHR> myFormats;		//交换链的表面格式
		std::vector<VkPresentModeKHR> myPresentModes;	//交换链的显示模式
	};

	class SwapChain {
	public:
		using Ptr = std::shared_ptr<SwapChain>;
		static Ptr create(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface) {
			return std::make_shared<SwapChain>(device, window, surface);
		}
		SwapChain(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface);

		~SwapChain();

		SwapChainSupportInfo querySwapChainSupportInfo();
		
		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& aviliableFromats);

		VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& aviliablePresentModes);

		//选择交换链中图像的大小
		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void createFrameBuffer(const RenderPass::Ptr &renderPass);

		[[nodiscard]] auto getFormat() const {
			return mySwapChainFormat;
		}

	private:
		//用于建立ImageView的辅助函数，输入图片，格式与ImageView管理的板块（颜色、深度...），和mipmap级别
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);
	
	private:
		VkSwapchainKHR mySwapChain{ VK_NULL_HANDLE };

		VkFormat mySwapChainFormat;
		VkExtent2D mySwapChainExtent;
		uint32_t myImageCount = 0;

		//vkimage由swapchain创建，亦由其销毁，ImageView用于管理Image
		std::vector<VkImage> mySwapChainImages{};
		std::vector<VkImageView> mySwapChainImageViews{};	

		std::vector<VkFramebuffer> mySwapChainFrameBuffers{};

		Device::Ptr myDevice{ nullptr };
		Window::Ptr myWindow{ nullptr };
		WindowSurface::Ptr mySurface{ nullptr };
	};

}