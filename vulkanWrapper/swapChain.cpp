#include "swapChain.h"

namespace FF::Wrapper {
	SwapChain::SwapChain(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface) {
		myDevice = device;
		myWindow = window;
		mySurface = surface;

		SwapChainSupportInfo swapChainSupportInfo = querySwapChainSupportInfo();

		//ѡ��vkformat
		VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.myFormats);
		//ѡ��presentMode
		VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.myPresentModes);
		//ѡ��extent
		VkExtent2D extent = chooseExtent(swapChainSupportInfo.myCapabilities);

		//����ͼ�񻺴�����
		myImageCount = swapChainSupportInfo.myCapabilities.minImageCount + 1;

		//��һ��Ҫ��ӳ��maxImageCountΪ0ʱ����ȡ����������ImageCount
		if (swapChainSupportInfo.myCapabilities.maxImageCount > 0 &&	
			myImageCount > swapChainSupportInfo.myCapabilities.maxImageCount) {
			myImageCount = swapChainSupportInfo.myCapabilities.maxImageCount;
		}

		//��д������Ϣ,���뽫������Ϣ��Ϊ�գ���Ϊ��Щ��������û���趨�����ÿջ�ʹ����Ϊ���ֵ
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mySurface->getSurface();
		createInfo.minImageCount = myImageCount;	//�����õ��������ʺϵ�ǰ���������ǿ��ܵõ�����
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;		//ͼ������Ĳ�Σ�VRһ���������������ۣ�
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	//���������ɵ�ͼ���ô����˴��Ǳ�ʾ��ɫ�����е����ʾ��Ȼ���ȵȣ�

		//��Ϊ��������ͼ�񣬻���������Ⱦ������ʾ������Ⱦ����ʾʹ�ò�һ���Ķ��У�
		//���Ի����ͬһ��ͼ����������ʹ�ã�����������Ҫ��ͼ���������������
		std::vector<uint32_t> queueFamilies = { myDevice->getGraphicQueueFamily().value(),myDevice->getPresentQueueFamily().value() };
		if (myDevice->getGraphicQueueFamily().value() == myDevice->getPresentQueueFamily().value()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	//���������Ӧͬһ��ʱ��ʹ�ö�ռ�����ã����ܸ���
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;	//������
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
			createInfo.pQueueFamilyIndices = queueFamilies.data();
		}

		//��������ͼ���ʼ���������Ƿ���Ҫ��ת
		createInfo.preTransform = swapChainSupportInfo.myCapabilities.currentTransform;

		//����ԭ���������ݻ��
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;

		//��ǰ���屻��ס�Ĳ��֣����û��ƣ����ǻ�Ӱ��ض�
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(myDevice->getDevice(), &createInfo, nullptr, &mySwapChain) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to creat swapChain");
		}
		
		mySwapChainFormat = surfaceFormat.format;
		mySwapChainExtent = extent;

		//ϵͳ���ܴ��������image����ǰ��ImageCount����С����
		vkGetSwapchainImagesKHR(myDevice->getDevice(), mySwapChain, &myImageCount, nullptr);
		mySwapChainImages.resize(myImageCount);

		vkGetSwapchainImagesKHR(myDevice->getDevice(), mySwapChain, &myImageCount, mySwapChainImages.data());

		//����Image View
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
		//��ȡ��������
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &info.myCapabilities);
	
		//��ȡ����֧�ָ�ʽ
		uint32_t fromatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &fromatCount, nullptr);
		if (fromatCount != 0) {
			info.myFormats.resize(fromatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &fromatCount, info.myFormats.data());
		}

		//��ȡ��ʾģʽ
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &presentModeCount, nullptr);
		if (fromatCount != 0) {
			info.myPresentModes.resize(fromatCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(myDevice->getPhysicalDevice(), mySurface->getSurface(), &presentModeCount, info.myPresentModes.data());
		}

		return info;
	}

	VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& aviliableFromats) {
		//��һ����������ֻ����һ��δ����ĸ�ʽ����ô��û����ѡ��ʽ�������Լ���һ��
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
		//���豸�ϣ�ֻ��FIFO�Ǳ�����֧�ֵģ���������ֻ����FIFO��һ�ָ�ʽ��û�и�����MAILBOX�ͷ�����ͨ�汾�汾
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& aviliablePresentMode : aviliablePresentModes) {
			if (aviliablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {		//ѡ��������ģʽ��ֱ�ӷ���
				return aviliablePresentMode;
			}
		}

		return bestMode;
	}

	VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		//����������������˵��ϵͳ��������������extent
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}

		//���ڸ�����Ļ�£�����������С�����������صĳ���
		int width = 0, height = 0;
		glfwGetFramebufferSize(myWindow->getWindow(), &width, &height);	//�õ�������Ϊ��λ�Ŀ��

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		//�涨��С��VkSurfaceCapabilitiesKHR�趨��max��min֮��
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

		viewInfo.subresourceRange.aspectMask = aspectFlags;		//ͼ���������
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
		//����FrameBuffer
		mySwapChainFrameBuffers.resize(myImageCount);
		for (int i = 0; i < myImageCount; ++i) {
			//FrameBuffer����һ֡�����ݣ����纬��n��ColorAttachment��1��DepthStencilAttachment��
			//��Щ�����ļ���Ϊһ��FrameBuffer��������߾ͻ��γ�һ��GPU���ϣ����Ϸ���Attachments����
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