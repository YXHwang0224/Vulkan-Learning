#pragma once

#include "../base.h"
#include "device.h"
#include "commandbuffer.h"
#include "commandpool.h"
#include "buffer.h"

namespace FF::Wrapper {

	//如果我们需要一张纹理采样的图片，需要先从undefineLayout变为TransferDst，
	//然后再数据拷贝完成后，再转换为ShaderReadOnly

	class Image {
	public:
		//静态的工具函数
		using Ptr = std::shared_ptr<Image>;
		static Ptr createDepthImage(
			const Device::Ptr device,
			const int& width,
			const int& height,
			VkSampleCountFlagBits samples
		);

		static Image::Ptr createRenderTargetImage(
			const Device::Ptr& device,
			const int& width,
			const int& height,
			VkFormat format
		);

	public:
		
		static Ptr create(
			const Device::Ptr device,
			const int& width,
			const int& height,
			const VkFormat& format,
			const VkImageType& imageType,
			const VkImageTiling& tiling,
			const VkImageUsageFlags& usage,
			const VkSampleCountFlagBits& samples,
			const VkMemoryPropertyFlags& properties,
			const VkImageAspectFlags& aspectFlags
		) {
			return std::make_shared<Image>(
				device, 
				width, 
				height, 
				format, 
				imageType, 
				tiling, 
				usage, 
				samples, 
				properties, 
				aspectFlags
			);
		}

		Image(
			const Device::Ptr device,
			const int& width,
			const int& height,
			const VkFormat& format,
			const VkImageType& imageType,
			const VkImageTiling& tiling,
			const VkImageUsageFlags& usage,
			const VkSampleCountFlagBits& samples,
			const VkMemoryPropertyFlags& properties,
			const VkImageAspectFlags& aspectFlags
		);

		~Image();

		void setImageLayout(
			VkImageLayout newLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange,
			const CommandPool::Ptr& commandPool
		);

		void fillImageData(size_t size,void* pData, const CommandPool::Ptr& commandPool);

		[[nodiscard]] VkImageLayout getLayout() {
			return myLayout;
		}
		[[nodiscard]] VkImage getImage() const {
			return myImage;
		}

		[[nodiscard]] size_t getWidth() const {
			return myWidth;
		}

		[[nodiscard]] size_t getHeight() const {
			return myHeight;
		}

		[[nodiscard]] VkImageView getImageView() const {
			return myImageView;
		}

	public:

		static VkFormat findDepthFormat(const Device::Ptr& device);
		
		static VkFormat findSupportedFormat(
			const Device::Ptr& device,
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		);

		bool hasStencilCompontent(VkFormat format);

	private:
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	private:
		size_t					myWidth;
		size_t					myHeight;
		
		Device::Ptr				myDevice{ nullptr };
		VkImage					myImage{ VK_NULL_HANDLE };
		VkDeviceMemory			myImageMemory{ VK_NULL_HANDLE };
		VkImageView				myImageView{ VK_NULL_HANDLE };
		VkImageLayout			myLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkFormat				myFormat{ VK_FORMAT_UNDEFINED };
	};
}