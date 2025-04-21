#include "swapChain.h"

namespace FF::Wrapper {
	SwapChain::SwapChain(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface) {
		myDevice = device;
		myWindow = window;
		mySurface = surface;

		SwapChainSupportInfo swapChainSupportInfo = querySwapChainSupportInfo();

		//选择vkformat
		VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.myFormats);
		//选择presentMode
		VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.myPresentModes);
		//选择extent
		VkExtent2D extent = chooseExtent(swapChainSupportInfo.myCapabilities);

		//设置图像缓存数量
		myImageCount = swapChainSupportInfo.myCapabilities.minImageCount + 1;

		//这一个要求反映了maxImageCount为0时，可取任意数量的ImageCount
		if (swapChainSupportInfo.myCapabilities.maxImageCount > 0 &&	
			myImageCount > swapChainSupportInfo.myCapabilities.maxImageCount) {
			myImageCount = swapChainSupportInfo.myCapabilities.maxImageCount;
		}

		//填写创建信息,必须将创建信息置为空，因为有些变量我们没有设定，不置空会使得其为随机值
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mySurface->getSurface();
		createInfo.minImageCount = myImageCount;	//我设置的数量，适合当前数量，但是可能得到更多
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;		//图像包含的层次，VR一般有两个（左右眼）
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	//交换链生成的图像用处（此处是表示颜色，还有的如表示深度缓存等等）

		//因为交换链的图像，会用来被渲染或者显示，而渲染和显示使用不一样的队列，
		//所以会出现同一个图像被两个队列使用，所以我们需要让图像对他们俩都兼容
		std::vector<uint32_t> queueFamilies = { myDevice->getGraphicQueueFamily().value(),myDevice->getPresentQueueFamily().value() };
		if (myDevice->getGraphicQueueFamily().value() == myDevice->getPresentQueueFamily().value()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	//当队列族对应同一个时，使用独占的设置，性能更好
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;	//被共享
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
			createInfo.pQueueFamilyIndices = queueFamilies.data();
		}

		//交换链的图像初始化，比如是否需要反转
		createInfo.preTransform = swapChainSupportInfo.myCapabilities.currentTransform;

		//不与原来窗体内容混合
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;

		//当前窗体被挡住的部分，不用绘制，但是会影响回读
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(myDevice->getDevice(), &createInfo, nullptr, &mySwapChain) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to creat swapChain");
		}
		
		mySwapChainFormat = surfaceFormat.format;
		mySwapChainExtent = extent;

		//系统可能创建更多的image，当前的ImageCount是最小数量
		vkGetSwapchainImagesKHR(myDevice->getDevice(), mySwapChain, &myImageCount, nullptr);
		mySwapChainImages.resize(myImageCount);

		vkGetSwapchainImagesKHR(myDevice->getDevice(), mySwapChain, &myImageCount, mySwapChainImages.data());

		//创建Image View
		mySwapChainImageViews.resize(myImageCount);
		for (int i = 0; i < myImageCount; ++i) {
			mySwapChainImageViews[i] = createImageView(mySwapChainImages[i], mySwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	SwapChain::~SwapChain() {
		for (auto& imageView : mySwapChainImageViews) {
			vkDestroyImageView(myDevice->getDevice(), imageView, nullptr);
		}

		for (auto& frameBuffer : mySwapChainFrameBuffers) {
			vkDestroyFramebuffer(myDevice->getDevice(), frameBuffer, nullptr);
		}

		if (mySwapChain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(myDevice->getDevice(), mySwapChain, nullptr);
			myWindow.reset();
			mySurface.reset();
			myDevice.reset();
		}
	}

	SwapChainSupportInfo SwapChain::querySwapChainSupportInfo() {
		SwapChainSupportInfo info;
		//获取基础特性
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &info.myCapabilities);
	
		//获取表面支持格式
		uint32_t fromatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &fromatCount, nullptr);
		if (fromatCount != 0) {
			info.myFormats.resize(fromatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &fromatCount, info.myFormats.data());
		}

		//获取显示模式
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &presentModeCount, nullptr);
		if (fromatCount != 0) {
			info.myPresentModes.resize(fromatCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &presentModeCount, info.myPresentModes.data());
		}

		return info;
	}

	VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& aviliableFromats) {
		//第一种情况，如果只返回一个未定义的格式，那么就没有首选格式，我们自己做一个
		if (aviliableFromats.size() == 1 && aviliableFromats[0].format == VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_B8G8R8A8_SRGB,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& aviliableFromat : aviliableFromats) {
			if (aviliableFromat.format == VK_FORMAT_B8G8R8A8_SRGB&& 
				aviliableFromat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return aviliableFromat;
			}
		}

		return aviliableFromats[0];
	}

	VkPresentModeKHR SwapChain::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& aviliablePresentModes) {
		//在设备上，只有FIFO是被绝对支持的，所以我们只考虑FIFO这一种格式，没有高性能MAILBOX就返回普通版本版本
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& aviliablePresentMode : aviliablePresentModes) {
			if (aviliablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {		//选到高性能模式，直接返回
				return aviliablePresentMode;
			}
		}

		return bestMode;
	}

	VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		//如果出现以下情况，说明系统不允许我们设置extent
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}

		//由于高清屏幕下，窗体的坐标大小，不等于像素的长度
		int width = 0, height = 0;
		glfwGetFramebufferSize(myWindow->getWindow(), &width, &height);	//得到以像素为单位的宽高

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		//规定大小在VkSurfaceCapabilitiesKHR设定的max和min之间
		actualExtent.width = std::max(capabilities.minImageExtent.width, 
			std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

	VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;

		viewInfo.subresourceRange.aspectMask = aspectFlags;		//图像解析分类
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView{ VK_NULL_HANDLE };
		if (vkCreateImageView(myDevice->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create image view in swapchain");
		}

		return imageView;
	}

	void SwapChain::createFrameBuffer(const RenderPass::Ptr& renderPass) {
		//创建FrameBuffer
		mySwapChainFrameBuffers.resize(myImageCount);
		for (int i = 0; i < myImageCount; ++i) {
			//FrameBuffer中是一帧的数据，比如含有n个ColorAttachment，1个DepthStencilAttachment，
			//这些东西的集合为一个FrameBuffer，送入管线就会形成一个GPU集合，由上方的Attachments构成
			std::array<VkImageView, 1>attachments = { mySwapChainImageViews[i] };

			VkFramebufferCreateInfo frameBufferCreateInfo{};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.renderPass = renderPass->getRenderPass();
			frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferCreateInfo.pAttachments = attachments.data();
			frameBufferCreateInfo.width = mySwapChainExtent.width;
			frameBufferCreateInfo.height = mySwapChainExtent.height;
			frameBufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(myDevice->getDevice(), &frameBufferCreateInfo, nullptr, &mySwapChainFrameBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Error: failed to create frameBufffer");
			}
		}
	}
}