#pragma once

#include "..\base.h"
#include "device.h"
#include "shader.h"
#include "renderpass.h"

namespace FF::Wrapper {
	class Pipeline {
	public:
		using Ptr = std::shared_ptr<Pipeline>;
		static Ptr create(const Device::Ptr& device, const RenderPass::Ptr& renderPass) {
			return std::make_shared<Pipeline>(device, renderPass);
		}

		Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass);

		~Pipeline();

		void setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup);

		void setViewports(const std::vector<VkViewport>& viewports) { myViewports = viewports; }

		void setScissors(const std::vector<VkRect2D>& scissors) { myScissors = scissors; }

		void pushBlendAttachment(const VkPipelineColorBlendAttachmentState& blendAttachment) {
			myBlendAttachmentState.push_back(blendAttachment);
		}

		void build();

		[[nodiscard]] auto getPipeline() const {
			return myPipeline;
		}
		[[nodiscard]] auto getLayout() const {
			return myLayout;
		}

	public:
		VkPipelineVertexInputStateCreateInfo myVertexInputState{};
		VkPipelineInputAssemblyStateCreateInfo myAssemblyState{};
		VkPipelineViewportStateCreateInfo myViewportState{};
		VkPipelineRasterizationStateCreateInfo myRasterState{};
		VkPipelineMultisampleStateCreateInfo mySampleState{};
		std::vector<VkPipelineColorBlendAttachmentState> myBlendAttachmentState{};
		VkPipelineColorBlendStateCreateInfo myBlendState{};
		VkPipelineDepthStencilStateCreateInfo myDepthStencilState{};
		VkPipelineLayoutCreateInfo myLayoutState{};

	private:
		VkPipeline myPipeline{ VK_NULL_HANDLE };
		VkPipelineLayout myLayout{ VK_NULL_HANDLE };
		Device::Ptr myDevice{ nullptr };
		RenderPass::Ptr myRenderPass{ nullptr };
		std::vector<Shader::Ptr> myShaders{};

		std::vector<VkViewport> myViewports{};
		std::vector<VkRect2D> myScissors{};
	};
}