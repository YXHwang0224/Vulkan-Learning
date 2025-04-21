#pragma once

#include "..\base.h"
#include "device.h"

namespace FF::Wrapper {

	class Shader {
	public:
		using Ptr = std::shared_ptr<Shader>;
		static Ptr create(const Device::Ptr device, const std::string& filename, VkShaderStageFlagBits shaderStage, const std::string& entryPoint) {
			return std::make_shared<Shader>(device, filename, shaderStage, entryPoint);
		}

		Shader(const Device::Ptr device, const std::string& filename, VkShaderStageFlagBits shaderStage, const std::string& entryPoint);

		~Shader();

		[[nodiscard]] auto getShaderStage() const { return myShaderStage; }
		[[nodiscard]] auto& getShaderEntryPoint() const { return myEntryPoint; }
		[[nodiscard]] auto getShaderModule() const { return myShaderModule; }

	private:
		VkShaderModule myShaderModule{ VK_NULL_HANDLE };
		Device::Ptr myDevice{ nullptr };
		std::string myEntryPoint;				//规定shader使用哪一个函数（main函数可能有多个）
		VkShaderStageFlagBits myShaderStage;	//shder的阶段（vertex、fragment）
	};
}