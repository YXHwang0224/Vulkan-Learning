#pragma once

#include "../base.h"
#include "device.h"
#include "window.h"
#include "windowSurface.h"
#include "renderpass.h"

namespace FF::Wrapper {

	struct SwapChainSupportInfo							//��������ص���Ϣ
	{
		VkSurfaceCapabilitiesKHR myCapabilities;		//�������Ļ�������
		std::vector<VkSurfaceFormatKHR> myFormats;		//�������ı����ʽ
		std::vector<VkPresentModeKHR> myPresentModes;	//����������ʾģʽ
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

		//ѡ�񽻻�����ͼ��Ĵ�С
		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void createFrameBuffer(const RenderPass::Ptr &renderPass);

		[[nodiscard]] auto getFormat() const {
			return mySwapChainFormat;
		}

	private:
		//���ڽ���ImageView�ĸ�������������ͼƬ����ʽ��ImageView����İ�飨��ɫ�����...������mipmap����
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);
	
	private:
		VkSwapchainKHR mySwapChain{ VK_NULL_HANDLE };

		VkFormat mySwapChainFormat;
		VkExtent2D mySwapChainExtent;
		uint32_t myImageCount = 0;

		//vkimage��swapchain���������������٣�ImageView���ڹ���Image
		std::vector<VkImage> mySwapChainImages{};
		std::vector<VkImageView> mySwapChainImageViews{};	

		std::vector<VkFramebuffer> mySwapChainFrameBuffers{};

		Device::Ptr myDevice{ nullptr };
		Window::Ptr myWindow{ nullptr };
		WindowSurface::Ptr mySurface{ nullptr };
	};

}