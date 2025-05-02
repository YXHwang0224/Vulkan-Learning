#pragma once

#include "../base.h"
#include "device.h"
#include "window.h"
#include "windowSurface.h"
#include "renderpass.h"

namespace FF::Wrapper {

	//基于Device得到交换链的三大特征（基础特征Capabilities、表面格式formats、显示模式PresentModes）
	//根据要求得到最后使用的表面格式、显示模式，再基于我们设定窗口和基础特性得到窗口大小extent
	//依据基础特性中对图片数量的限制得到最小图片数，再从device中得到最终需要的图片数量并以此创建ImageView

	//而后根据以上信息配以交换链图片在多队列中共享方式以及其它信息建立交换链

	//此外，在交换链类中定义了基于指定交换链建立帧缓冲的方式，每个ImageView作为附件建立一个帧缓冲

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
		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);		//选择交换链中图像的大小

		void createFrameBuffer(const RenderPass::Ptr &renderPass);

		[[nodiscard]] auto getFormat() const {
			return mySwapChainFormat;
		}

		[[nodiscard]] auto getImageCount() const {
			return myImageCount;
		}

		[[nodiscard]] auto getSwapChain() const {
			return mySwapChain;
		}

		[[nodiscard]] auto getFrameBuffer(const int index) const {
			return mySwapChainFrameBuffers[index];
		}

		[[nodiscard]] auto getExtent() const {
			return mySwapChainExtent;
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