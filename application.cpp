#include "application.h"
#include "vulkanWrapper/image.h"


namespace FF {

	void Application::run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::onMouseMove(double xpos, double ypos) {
		myCamera.onMouseMove(xpos, ypos);
	}

	void Application::onKeyDown(CAMERA_MOVE moveDirection) {
		myCamera.move(moveDirection);
	}

	void Application::initWindow() {
		myWindow = Wrapper::Window::create(myWidth, myHeight);
		myWindow->setApplication(shared_from_this());

		myCamera.lookAt(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		myCamera.update();

		myCamera.setPerpective(45.0f, (float)myWidth / (float)myHeight, 0.1f, 100.0f);

		myCamera.setSpeed(0.05f);
	}

	void Application::initVulkan() {
		myInstance = Wrapper::Instance::create(true);
		mySurface = Wrapper::WindowSurface::create(myInstance, myWindow);

		myDevice = Wrapper::Device::create(myInstance, mySurface);

		myCommandPool = Wrapper::CommandPool::create(myDevice);

		mySwapChain = Wrapper::SwapChain::create(myDevice, myWindow, mySurface, myCommandPool);
		myWidth = mySwapChain->getExtent().width;
		myHeight = mySwapChain->getExtent().height;

		myRenderPass = Wrapper::RenderPass::create(myDevice);
		createRenderPass();

		mySwapChain->createFrameBuffer(myRenderPass);



		//descriptor ===========================
		myUniformManager = UniformManager::create();
		myUniformManager->init(myDevice, myCommandPool, mySwapChain->getImageCount());

		//创建模型
		myModel = Model::create(myDevice);
		myModel->loadModel("assets/book/book.obj", myDevice);

		myPipeline = Wrapper::Pipeline::create(myDevice, myRenderPass);
		createPipeline();

		myCommandBuffers.resize(mySwapChain->getImageCount());

		createCommandBuffers();

		createSemaphore();


	}

	void Application::createPipeline() {
		//设置视口
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)myHeight;
		viewport.width = (float)myWidth;
		viewport.height = -(float)myHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { myWidth, myHeight };

		myPipeline->setViewports({ viewport });
		myPipeline->setScissors({ scissor });


		//设置shader
		std::vector<Wrapper::Shader::Ptr> shaderGroup{};

		auto shaderVertex = Wrapper::Shader::create(myDevice, "shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		shaderGroup.push_back(shaderVertex);

		auto shaderFragment = Wrapper::Shader::create(myDevice, "shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		shaderGroup.push_back(shaderFragment);

		myPipeline->setShaderGroup(shaderGroup);

		//顶点的排布模式
		auto vertexBindingDes = myModel->getVertexInputBindingDescriptions();
		auto attributeDes = myModel->getAttributeDescriptions();

		myPipeline->myVertexInputState.vertexBindingDescriptionCount = vertexBindingDes.size();
		myPipeline->myVertexInputState.pVertexBindingDescriptions = vertexBindingDes.data();
		myPipeline->myVertexInputState.vertexAttributeDescriptionCount = attributeDes.size();
		myPipeline->myVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

		//图元装配
		myPipeline->myAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		myPipeline->myAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		myPipeline->myAssemblyState.primitiveRestartEnable = VK_FALSE;

		//光栅化设置
		myPipeline->myRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		myPipeline->myRasterState.polygonMode = VK_POLYGON_MODE_FILL;//其他模式需要开启gpu特性
		myPipeline->myRasterState.lineWidth = 1.0f;//大于1需要开启gpu特性
		myPipeline->myRasterState.cullMode = VK_CULL_MODE_BACK_BIT;
		myPipeline->myRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		myPipeline->myRasterState.depthBiasEnable = VK_FALSE;
		myPipeline->myRasterState.depthBiasConstantFactor = 0.0f;
		myPipeline->myRasterState.depthBiasClamp = 0.0f;
		myPipeline->myRasterState.depthBiasSlopeFactor = 0.0f;

		//多重采样
		myPipeline->mySampleState.sampleShadingEnable = VK_FALSE;
		myPipeline->mySampleState.rasterizationSamples = myDevice->getMaxUsableSampleCount();
		myPipeline->mySampleState.minSampleShading = 1.0f;
		myPipeline->mySampleState.pSampleMask = nullptr;
		myPipeline->mySampleState.alphaToCoverageEnable = VK_FALSE;
		myPipeline->mySampleState.alphaToOneEnable = VK_FALSE;

		//深度与模板测试
		myPipeline->myDepthStencilState.depthTestEnable = VK_TRUE;
		myPipeline->myDepthStencilState.depthWriteEnable = VK_TRUE;
		myPipeline->myDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		//颜色混合

		//这个是颜色混合掩码，得到的混合结果，按照通道与掩码进行AND操作，输出
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		myPipeline->pushBlendAttachment(blendAttachment);

		//1 blend有两种计算方式，第一种如上所述，进行alpha为基础的计算，第二种进行位运算
		//2 如果开启了logicOp，那么上方设置的alpha为基础的运算，失灵
		//3 ColorWrite掩码，仍然有效，即便开启了logicOP
		//4 因为，我们可能会有多个FrameBuffer输出，所以可能需要多个BlendAttachment
		myPipeline->myBlendState.logicOpEnable = VK_FALSE;
		myPipeline->myBlendState.logicOp = VK_LOGIC_OP_COPY;

		//配合blendAttachment的factor与operation
		myPipeline->myBlendState.blendConstants[0] = 0.0f;
		myPipeline->myBlendState.blendConstants[1] = 0.0f;
		myPipeline->myBlendState.blendConstants[2] = 0.0f;
		myPipeline->myBlendState.blendConstants[3] = 0.0f;

		//uniform的传递
		myPipeline->myLayoutState.setLayoutCount = 1;

		auto layout = myUniformManager->getDescriptorLayout()->getLayout();
		myPipeline->myLayoutState.pSetLayouts = &layout;
		myPipeline->myLayoutState.pushConstantRangeCount = 0;
		myPipeline->myLayoutState.pPushConstantRanges = nullptr;

		myPipeline->build();
	}

	void Application::createRenderPass() {
		//0：最终输出图片 1：Resolve图片（MutiSample） 2：Depth图片

		//0号位：是SwapChain原来那张图片，是Resolve的目标点，即需要设置到SubPass的Resolve当中
		VkAttachmentDescription finalAttachmentDes{};
		finalAttachmentDes.format = mySwapChain->getFormat();
		finalAttachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
		finalAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		finalAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		finalAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		finalAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		finalAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		finalAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		myRenderPass->addAttachment(finalAttachmentDes);

		//1号位：被Resolve的图片，即多重采样的源头图片，也即颜色输出的目标图片
		VkAttachmentDescription MutiAttachmentDes{};
		MutiAttachmentDes.format = mySwapChain->getFormat();
		MutiAttachmentDes.samples = myDevice->getMaxUsableSampleCount();
		MutiAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		MutiAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		MutiAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		MutiAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		MutiAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		MutiAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		myRenderPass->addAttachment(MutiAttachmentDes);


		//3号位：深度缓存attachment
		VkAttachmentDescription depthAttachmentDes{};
		depthAttachmentDes.format = Wrapper::Image::findDepthFormat(myDevice);
		depthAttachmentDes.samples = myDevice->getMaxUsableSampleCount();
		depthAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		myRenderPass->addAttachment(depthAttachmentDes);

		//对于画布的索引设置以及格式要求
		VkAttachmentReference finalAttachmentRef{};
		finalAttachmentRef.attachment = 0;
		finalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference mutiAttachmentRef{};
		mutiAttachmentRef.attachment = 1;
		mutiAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 2;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//创建子流程
		Wrapper::SubPass subPass{};
		subPass.addColorAttachmentReference(mutiAttachmentRef);
		subPass.setDepthStencilAttachmentReference(depthAttachmentRef);
		subPass.setResolveAttachmentReference(finalAttachmentRef);

		subPass.buildSubPassDescription();

		myRenderPass->addSubPass(subPass);

		//子流程之间的依赖关系
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		myRenderPass->addDependency(dependency);

		myRenderPass->buildRenderPass();
	}

	void Application::createCommandBuffers() {
		for (int i = 0; i < mySwapChain->getImageCount(); ++i) {
			myCommandBuffers[i] = Wrapper::CommandBuffer::create(myDevice, myCommandPool);

			myCommandBuffers[i]->begin();

			VkRenderPassBeginInfo renderBeginInfo{};
			renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderBeginInfo.renderPass = myRenderPass->getRenderPass();
			renderBeginInfo.framebuffer = mySwapChain->getFrameBuffer(i);
			renderBeginInfo.renderArea.offset = { 0, 0 };
			renderBeginInfo.renderArea.extent = mySwapChain->getExtent();

			//0：final   1：muti   2：depth
			std::vector< VkClearValue> clearColors{};
			VkClearValue finalClearColor{};
			finalClearColor.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			clearColors.push_back(finalClearColor);

			VkClearValue mutiClearColor{};
			mutiClearColor.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			clearColors.push_back(mutiClearColor);

			VkClearValue depthClearColor{};
			depthClearColor.depthStencil = { 1.0f, 0 };
			clearColors.push_back(depthClearColor);

			renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
			renderBeginInfo.pClearValues = clearColors.data();


			myCommandBuffers[i]->beginRenderPass(renderBeginInfo);

			myCommandBuffers[i]->bindGraphicPipeline(myPipeline->getPipeline());

			myCommandBuffers[i]->bindDescriptorSet(myPipeline->getLayout(), myUniformManager->getDescriptorSet(myCurrentFrame));

			//myCommandBuffers[i]->bindVertexBuffer({ myModel->getVertexBuffer()->getBuffer() });

			myCommandBuffers[i]->bindVertexBuffer(myModel->getVertexBuffers());

			myCommandBuffers[i]->bindIndexBuffer(myModel->getIndexBuffer()->getBuffer());

			myCommandBuffers[i]->drawIndex(myModel->getIndexCount());

			myCommandBuffers[i]->endRenderPass();

			myCommandBuffers[i]->end();
		}
	}

	void Application::createSemaphore() {
		for (int i = 0; i < mySwapChain->getImageCount(); ++i) {
			auto imageSemaphore = Wrapper::Semaphore::create(myDevice);
			myImageAvailableSemaphores.push_back(imageSemaphore);

			auto renderSemaphore = Wrapper::Semaphore::create(myDevice);
			myRenderFinishedSemaphores.push_back(renderSemaphore);

			auto fence = Wrapper::Fence::create(myDevice);
			myFences.push_back(fence);
		}
	}

	void Application::recreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(myWindow->getWindow(), &width, &height);
		while (width == 0 || height == 0) {
			glfwWaitEvents();
			glfwGetFramebufferSize(myWindow->getWindow(), &width, &height);
		}

		vkDeviceWaitIdle(myDevice->getDevice());

		cleanupSwapChain();

		mySwapChain = Wrapper::SwapChain::create(myDevice, myWindow, mySurface, myCommandPool);
		myWidth = mySwapChain->getExtent().width;
		myHeight = mySwapChain->getExtent().height;

		myRenderPass = Wrapper::RenderPass::create(myDevice);
		createRenderPass();

		mySwapChain->createFrameBuffer(myRenderPass);

		myPipeline = Wrapper::Pipeline::create(myDevice, myRenderPass);
		createPipeline();

		myCommandBuffers.resize(mySwapChain->getImageCount());

		createCommandBuffers();

		createSemaphore();
	}

	void Application::cleanupSwapChain() {
		mySwapChain.reset();
		myCommandBuffers.clear();
		myPipeline.reset();
		myRenderPass.reset();
		myImageAvailableSemaphores.clear();
		myRenderFinishedSemaphores.clear();
		myFences.clear();
	}


	void Application::mainLoop() {
		while (!myWindow->shouldClose()) {
			myWindow->pollEvents();
			myWindow->processEvents();

			//myModel->update();

			myVPMatrices.myViewMatrix = myCamera.getViewMatrix();
			myVPMatrices.myProjectionMatrix = myCamera.getProjectMatrix();


			myUniformManager->update(myVPMatrices, myModel->getUniform(), myCurrentFrame);

			render();
		}

		vkDeviceWaitIdle(myDevice->getDevice());
	}

	void Application::render() {
		//等待当前要提交的CommandBuffer执行完毕
		myFences[myCurrentFrame]->block();

		//获取交换链当中的下一帧
		uint32_t imageIndex{ 0 };
		VkResult result = vkAcquireNextImageKHR(
			myDevice->getDevice(),
			mySwapChain->getSwapChain(),
			UINT64_MAX,
			myImageAvailableSemaphores[myCurrentFrame]->getSemaphore(),
			VK_NULL_HANDLE,
			&imageIndex);

		//窗体发生了尺寸变化
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			myWindow->myWindowResized = false;
		}//VK_SUBOPTIMAL_KHR得到了一张认为可用的图像，但是表面格式不一定匹配
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Error: failed to acquire next image");
		}

		//构建提交信息
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;


		//同步信息，渲染对于显示图像的依赖，显示完毕后，才能输出颜色
		VkSemaphore waitSemaphores[] = { myImageAvailableSemaphores[myCurrentFrame]->getSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//指定提交哪些命令
		auto commandBuffer = myCommandBuffers[imageIndex]->getCommandBuffer();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = { myRenderFinishedSemaphores[myCurrentFrame]->getSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		myFences[myCurrentFrame]->resetFence();
		if (vkQueueSubmit(myDevice->getGraphicQueue(), 1, &submitInfo, myFences[myCurrentFrame]->getFence()) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to submit renderCommand");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mySwapChain->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(myDevice->getPresentQueue(), &presentInfo);

		//由于驱动程序不一定精准，所以我们还需要用自己的标志位判断
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || myWindow->myWindowResized) {
			recreateSwapChain();
			myWindow->myWindowResized = false;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to present");
		}

		myCurrentFrame = (myCurrentFrame + 1) % mySwapChain->getImageCount();
	}

	void Application::cleanUp() {

		myPipeline.reset();

		myRenderPass.reset();

		mySwapChain.reset();

		myDevice.reset();
		mySurface.reset();
		myInstance.reset();
		myWindow.reset();
	}
}