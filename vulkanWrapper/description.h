#pragma once

#include "../base.h"
#include "buffer.h"
#include "../texture/texture.h"

namespace FF::Wrapper {

	//需要知道，布局中到底有哪些uniform，每一个多大，如何binding，每一个是什么类型
	struct UniformParameter {
		using Ptr = std::shared_ptr<UniformParameter>;
		static Ptr create() {
			return std::make_shared<UniformParameter>();
		}

		size_t mySize{ 0 };
		uint32_t myBinding{ 0 };

		//对于一个binding点，可能传入多个uniform，这个count就代表数组的大小
		//这种以数组为单位传入需要使用IndexDescriptor类型
		uint32_t myCount{ 0 };
		VkDescriptorType myDescriptorType;
		VkShaderStageFlagBits myStage;

		std::vector<Buffer::Ptr> myBuffers{};	//表示这个uniform在哪些buffer中存在
		Texture::Ptr myTexture;		//表示这个uniform是从texture中读取的
	};
}