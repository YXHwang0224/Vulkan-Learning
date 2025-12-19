#include "image.h"

namespace FF::Wrapper {

	Image::Ptr Image::createDepthImage(
		const Device::Ptr device,
		const int& width,
		const int& height,
		VkSampleCountFlagBits samples
	) {
		std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};

		VkFormat resultFormat = findSupportedFormat(
			device, depthFormats, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		return Image::create(
			device, width, height,
			resultFormat,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			samples,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT
		);
	}

	Image::Ptr Image::createRenderTargetImage(
		const Device::Ptr& device,
		const int& width,
		const int& height,
		VkFormat format
	) {
		return Image::create(
			device,
			width, height,
			format,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,

			//VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT，当图片拥有这个标识的时候
			//只有真正使用到本图片的时候，才会为其创建内存，显性的调用内存生成，是会lazy的
			//所以，一旦设置了transient，就必须在内存生成的时候DeviceMemory，加一个参数叫做lazy
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			device->getMaxUsableSampleCount(),

			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			//注意，如果上方用了transient，那么这里就需要与运算一个VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT

			VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	Image::Image(
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
		myDevice = device;
		myLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		myWidth= width;
		myHeight = height;
		myFormat = format;

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.format = format;
		imageCreateInfo.imageType = imageType;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.usage = usage;
		imageCreateInfo.samples = samples;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(myDevice->getDevice(), &imageCreateInfo, nullptr, &myImage) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create image");
		}

		//分配内存空间
		VkMemoryRequirements memoryRequirements{};
		vkGetImageMemoryRequirements(myDevice->getDevice(), myImage, &memoryRequirements);

		//分配显存
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;

		//符合上述buffer要求的内存类型的ID集合
		allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);
		
		if (vkAllocateMemory(myDevice->getDevice(), &allocateInfo, nullptr, &myImageMemory) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to allocate memory");
		}

		vkBindImageMemory(myDevice->getDevice(), myImage, myImageMemory, 0);

		//创建imageview
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = imageType == VK_IMAGE_TYPE_2D ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
		imageViewCreateInfo.image = myImage;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(myDevice->getDevice(), &imageViewCreateInfo, nullptr, &myImageView) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create image view");
		}
	}	

	Image::~Image() {
		if (myImage != VK_NULL_HANDLE) {
			vkDestroyImage(myDevice->getDevice(), myImage, nullptr);
		}
		if (myImageView != VK_NULL_HANDLE) {
			vkDestroyImageView(myDevice->getDevice(), myImageView, nullptr);
		}
		if (myImageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(myDevice->getDevice(), myImageMemory, nullptr);
		}
	}

	uint32_t Image::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(myDevice->getPhysicalDevice(), &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
			//前面一部分是检验内存是否满足要求，后面一部分是检验是否满足特性需求
			if (typeFilter & (1 << i) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
				return i;
			}
		}

		throw std::runtime_error("Error: failed to the property memory type");
	}

	VkFormat Image::findDepthFormat(const Device::Ptr& device) {
		std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};

		return findSupportedFormat(
			device, depthFormats,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

	}

	VkFormat Image::findSupportedFormat(
		const Device::Ptr& device, 
		const std::vector<VkFormat>& candidates, 
		VkImageTiling tiling, 
		VkFormatFeatureFlags features
	) {
		for (auto format : candidates) {
			VkFormatProperties porps;
			vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &porps);
			if (tiling == VK_IMAGE_TILING_LINEAR && (porps.linearTilingFeatures & features) == features) {
				return format;
			}

			if (tiling == VK_IMAGE_TILING_OPTIMAL && (porps.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("Error: failed to find supported format");
	}

	bool Image::hasStencilCompontent(VkFormat format) {
		return myFormat == VK_FORMAT_D24_UNORM_S8_UINT || myFormat == VK_FORMAT_D32_SFLOAT_S8_UINT;
	}

	void Image::setImageLayout(
		VkImageLayout newLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange,
		const CommandPool::Ptr& commandPool
	) {
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.oldLayout = myLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = myImage;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		switch (myLayout)
		{
			//如果是无定义，说明图片刚创建，上方没有任何操作，是一个虚拟的依赖，不必关系上一阶段的任何操作
		case VK_IMAGE_LAYOUT_UNDEFINED:
			imageMemoryBarrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		default:
			break;
		}

		switch (newLayout)
		{
			//如果目标是将图片转变为一个复制操作的目标图片/内存，那么被阻塞的操作一定是写入操作
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
			//如果目标是将图片转变为一个适合作为纹理的格式，那么被阻塞的一定是读取操作
			//如果目标是texture，那么只有两种来源，一是从CPU来，而是从staging buffer拷贝而来
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
			if (imageMemoryBarrier.srcAccessMask == 0) {
				imageMemoryBarrier.srcAccessMask= VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: {
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
			break;
		default:
			break;
		}

		myLayout = newLayout;
		auto commandBuffer = CommandBuffer::create(myDevice, commandPool);
		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->transferImageLayout(imageMemoryBarrier, srcStageMask, dstStageMask);
		commandBuffer->end();

		commandBuffer->submitSync(myDevice->getGraphicQueue());
	}

	void Image::fillImageData(size_t size, void* pData, const CommandPool::Ptr& commandPool) {
		assert(pData);
		assert(size);

		auto stageBuffer = Buffer::createStageBuffer(myDevice, size, pData);

		auto commandBuffer = CommandBuffer::create(myDevice, commandPool);
		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->copyBufferToImage(stageBuffer->getBuffer(), myImage, myLayout, myWidth, myHeight);
		commandBuffer->end();

		commandBuffer->submitSync(myDevice->getGraphicQueue());
	}
}