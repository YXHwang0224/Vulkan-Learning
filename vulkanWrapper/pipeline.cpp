#include "pipeline.h"

namespace FF::Wrapper {
	Pipeline::Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass) {
		myDevice = device;
		myRenderPass = renderPass;

		myVertexInputState.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		myAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		myViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		myRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mySampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		myBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		myDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		myLayoutState.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	}

	Pipeline::~Pipeline() {
		if (myLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(myDevice->getDevice(), myLayout, nullptr);
		}

		if (myPipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(myDevice->getDevice(), myPipeline, nullptr);
		}
	}

	void Pipeline::setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup) {
		myShaders = shaderGroup;
	}

	void Pipeline::build() {
		//设置shader
		std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
		for (const auto& shader : myShaders) {
			VkPipelineShaderStageCreateInfo shaderCreateInfo{};
			shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderCreateInfo.stage = shader->getShaderStage();
			shaderCreateInfo.pName = shader->getShaderEntryPoint().c_str();
			shaderCreateInfo.module = shader->getShaderModule();

			shaderCreateInfos.push_back(shaderCreateInfo);
		}

		//设置视口剪裁
		myViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		myViewportState.viewportCount = static_cast<uint32_t>(myViewports.size());
		myViewportState.pViewports = myViewports.data();
		myViewportState.scissorCount = static_cast<uint32_t>(myScissors.size());
		myViewportState.pScissors = myScissors.data();

		//blending的基本设置，在pipeline类中设置，不暴露接口
		myBlendState.attachmentCount = static_cast<uint32_t>(myBlendAttachmentState.size());
		myBlendState.pAttachments = myBlendAttachmentState.data();

		//layout生成
		if (myLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(myDevice->getDevice(), myLayout, nullptr);
		}

		if (vkCreatePipelineLayout(myDevice->getDevice(), &myLayoutState, nullptr, &myLayout) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create pipeline layout");
		}

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderCreateInfos.size());
		pipelineCreateInfo.pStages = shaderCreateInfos.data();
		
		pipelineCreateInfo.pVertexInputState = &myVertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &myAssemblyState;
		pipelineCreateInfo.pViewportState = &myViewportState;
		pipelineCreateInfo.pRasterizationState = &myRasterState;
		pipelineCreateInfo.pMultisampleState = &mySampleState;
		pipelineCreateInfo.pDepthStencilState = nullptr;
		pipelineCreateInfo.pColorBlendState = &myBlendState;
		pipelineCreateInfo.layout = myLayout;
		pipelineCreateInfo.renderPass = myRenderPass->getRenderPass();
		pipelineCreateInfo.subpass = 0;
		
		//以存在的pipeline为基础进行创建，会更快，但是需要指定flags
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = -1;

		//pipeline cache：可以将相关文件存入缓冲，在多个pipeline中使用，也可以存到文件不同程序调用
		if (myPipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(myDevice->getDevice(), myPipeline, nullptr);
		}
		if (vkCreateGraphicsPipelines(myDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &myPipeline) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create pipeline");
		}
	}
}