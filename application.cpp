#include "application.h"

namespace FF {

	void Application::Run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::initWindow() {
		myWindow = Wrapper::Window::create(myWidth, myHeight);
	}

	void Application::initVulkan() {
		myInstance = Wrapper::Instance::create(true);
		mySurface = Wrapper::WindowSurface::create(myInstance, myWindow);

		myDevice = Wrapper::Device::create(myInstance, mySurface);

		mySwapChain = Wrapper::SwapChain::create(myDevice, myWindow, mySurface);
		myWidth = mySwapChain->getExtent().width;
		myHeight = mySwapChain->getExtent().height;

		myRenderPass = Wrapper::RenderPass::create(myDevice);
		createRenderPass();

		mySwapChain->createFrameBuffer(myRenderPass);

		myUniformManger = UniformManager::create();
		myUniformManger->init(myDevice, mySwapChain->getImageCount());

		myModel = Model::create(myDevice);

		myPipeline = Wrapper::Pipeline::create(myDevice, myRenderPass);
		createPipeline();

		myCommandPool = Wrapper::CommandPool::create(myDevice);

		myCommandBuffers.resize(mySwapChain->getImageCount());
		createCommandBuffer();

		createSemaphore();
	}

	void Application::mainLoop() {
		while (!myWindow->shouldClose()) {
			myWindow->pollEvents();		//分发鼠标键盘等信息

			myModel->update();

			myUniformManger->update(myVPMatrices, myModel->getUniform(), myCurrentFrame);

			render();
		}

		//等待device所有操作结束后再清理
		vkDeviceWaitIdle(myDevice->getDevice());
	}

	void Application::render() {
		//等待当前要提交的commandbuffer执行完毕
		myFences[myCurrentFrame]->block();

		//首先获取交换链的下一帧
		uint32_t imageIndex{ 0 };
		VkResult result = vkAcquireNextImageKHR(		//第三个参数为超时时间
			myDevice->getDevice(),
			mySwapChain->getSwapChain(),
			UINT64_MAX,
			myImageAvaliablesemaphores[myCurrentFrame]->getsemaphore(),
			VK_NULL_HANDLE,
			&imageIndex);

		//检测窗体发生尺寸变换
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			myWindow->myWindowResized = false;
		}
		//VK_SUBOPTIMAL_KHR：得到了一张认为可用的图像，但是表面格式并不完全匹配
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Error: failes to acquire next image");
		}

		//构建提交信息
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//同步信息，渲染对于显示图像的依赖，显示完毕后才能输出颜色
		VkSemaphore waitsemaphores[] = { myImageAvaliablesemaphores[myCurrentFrame]->getsemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitsemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//指定提交哪些命令
		auto commandBuffer = myCommandBuffers[imageIndex]->getCommandBuffer();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		//执行完毕后激发哪些semaphore
		VkSemaphore signalsemaphore[] = { myRenderFinishedsemaphores[myCurrentFrame]->getsemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalsemaphore;

		myFences[myCurrentFrame]->resetFence();

		if (vkQueueSubmit(myDevice->getGraphicQueue(), 1, &submitInfo, myFences[myCurrentFrame]->getFence()) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to submit render command");
		}

		//构建显示信息
		VkPresentInfoKHR presentinfo{};
		presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentinfo.waitSemaphoreCount = 1;
		presentinfo.pWaitSemaphores = signalsemaphore;

		//定义显示时使用的交换链信息
		VkSwapchainKHR swapChains[] = { mySwapChain->getSwapChain() };
		presentinfo.swapchainCount = 1;
		presentinfo.pSwapchains = swapChains;

		presentinfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(myDevice->getPresentQueue(), &presentinfo);

		//由于驱动程序不一定精确，所以我们还需要使用Window中定义的标志位进行判断
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
		myWindow->~Window();
	}


	void Application::createPipeline() {
		//设置视口
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)myWidth;
		viewport.height = (float)myHeight;
		//在屏幕坐标中，z轴范围为0到1
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//设置剪裁
		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = { myWidth, myHeight };

		//设置视口和剪裁至pipeline中
		myPipeline->setViewports({ viewport });
		myPipeline->setScissors({ scissor });

		//设置shader
		std::vector<Wrapper::Shader::Ptr> shaderGroup{};
		auto vertexShader = Wrapper::Shader::create(myDevice, "shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		shaderGroup.push_back(vertexShader);
		auto fragmentShader = Wrapper::Shader::create(myDevice, "shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		shaderGroup.push_back(fragmentShader);

		myPipeline->setShaderGroup(shaderGroup);

		//顶点排布模式
		auto vertexBindingDescription = myModel->getVertexInputBindingDescription();
		auto attributeDescription = myModel->getAttributeDescription();

		myPipeline->myVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescription.size());
		myPipeline->myVertexInputState.pVertexBindingDescriptions = vertexBindingDescription.data();
		myPipeline->myVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		myPipeline->myVertexInputState.pVertexAttributeDescriptions = attributeDescription.data();

		//图元装配
		myPipeline->myAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		myPipeline->myAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		//设置图元拓扑结构，形成的图元
		myPipeline->myAssemblyState.primitiveRestartEnable = VK_FALSE;			//不重启图元

		//光栅化设置
		myPipeline->myRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		myPipeline->myRasterState.polygonMode = VK_POLYGON_MODE_FILL;		//解释光栅化图元选项，其他选项需要开启GPU特性
		myPipeline->myRasterState.lineWidth = 1.0f;		//大于1需要开启GPU特性
		myPipeline->myRasterState.cullMode = VK_CULL_MODE_BACK_BIT;			//背面剔除
		myPipeline->myRasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;		//定义正反面

		myPipeline->myRasterState.depthBiasEnable = VK_FALSE;			//改变深度信息
		myPipeline->myRasterState.depthBiasConstantFactor = 0.0f;
		myPipeline->myRasterState.depthBiasClamp = 0.0f;
		myPipeline->myRasterState.depthBiasSlopeFactor = 0.0f;

		//多重采样
		myPipeline->mySampleState.sampleShadingEnable = VK_FALSE;
		myPipeline->mySampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		myPipeline->mySampleState.minSampleShading = 1.0f;
		myPipeline->mySampleState.pSampleMask = nullptr;
		myPipeline->mySampleState.alphaToCoverageEnable = VK_FALSE;
		myPipeline->mySampleState.alphaToOneEnable = VK_FALSE;

		//深度与模板测试


		//颜色混合


		//这个是颜色混合掩码，得到的混合结果，按照通道与掩码进行AND操作，输出
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	//这个参数表示将目标颜色与某种颜色相乘后输出，使用该参数设置表示直接使用原结果输出
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	//表示不混合
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;		//表示混合运算以相加进行

		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		myPipeline->pushBlendAttachment(blendAttachment);

		//blending的差异化接口，在application类中设定
		//1.blend有两种方式，其一如上，进行alpha为基础的运算，第二种为逻辑位运算，用图像本来的结果叠加处理
		//2.如果开启了logicOp，那么上方设置的alpha为基础的运算失效
		//3.ColorWrite掩码，仍然有效，即使开启了logicOp
		//4.因为我们可能会有多个FrameBuffer输出，所以可能需要多个BlendAttachment
		myPipeline->myBlendState.logicOpEnable = VK_FALSE;
		myPipeline->myBlendState.logicOp = VK_LOGIC_OP_COPY;

		//配合blendAttachment的factor与operation
		myPipeline->myBlendState.blendConstants[0] = 0.0f;
		myPipeline->myBlendState.blendConstants[1] = 0.0f;
		myPipeline->myBlendState.blendConstants[2] = 0.0f;
		myPipeline->myBlendState.blendConstants[3] = 0.0f;

		//uniform的传递
		myPipeline->myLayoutState.setLayoutCount = 1;

		auto layout = myUniformManger->getDescriptorLayout()->getLayout();
		myPipeline->myLayoutState.pSetLayouts = &layout;
		myPipeline->myLayoutState.pushConstantRangeCount = 0;
		myPipeline->myLayoutState.pPushConstantRanges = nullptr;

		myPipeline->build();
	}

	void Application::createRenderPass() {
		//输入画布的描述
		VkAttachmentDescription attachmentDes{};
		attachmentDes.format = mySwapChain->getFormat();
		attachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;		//输入时如果有残余数据，则清空
		attachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;	//输出时如果有残余数据，则保留
		attachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	//最终出RenderPass时的格式

		myRenderPass->addAttachment(attachmentDes);

		//对于画布的索引设置及格式要求
		VkAttachmentReference attachmentReference{};
		attachmentReference.attachment = 0;
		attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	//表示GPU在将该附着加入SubPass时将其转为什么格式

		//创建SubPass
		Wrapper::SubPass subPass{};
		subPass.addColorAttachmentReference(attachmentReference);
		subPass.buildSubPassDesciption();

		myRenderPass->addSubPass(subPass);

		//SubPass的依赖关系
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;	//上一个流程为虚拟流程
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	//上一个subpass输出到画布上时开启下一个
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		myRenderPass->addDependency(dependency);

		myRenderPass->buildRenderPass();
	}

	void Application::createCommandBuffer() {
		myCommandBuffers.resize(mySwapChain->getImageCount());

		for (int i = 0; i < mySwapChain->getImageCount(); ++i) {
			myCommandBuffers[i] = Wrapper::CommandBuffer::create(myDevice, myCommandPool);

			myCommandBuffers[i]->begin();

			VkRenderPassBeginInfo renderBeginInfo{};
			renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderBeginInfo.renderPass = myRenderPass->getRenderPass();
			renderBeginInfo.framebuffer = mySwapChain->getFrameBuffer(i);
			renderBeginInfo.renderArea.offset = { 0,0 };	//绘制起始点，从（0，0）点开始
			renderBeginInfo.renderArea.extent = mySwapChain->getExtent();

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f };	//每帧绘制前，使用何种颜色清理画布
			renderBeginInfo.clearValueCount = 1;
			renderBeginInfo.pClearValues = &clearColor;

			myCommandBuffers[i]->beginRenderPass(renderBeginInfo);
			myCommandBuffers[i]->bindGraphicPipeline(myPipeline->getPipeline());
			myCommandBuffers[i]->bindDescriptorSet(myPipeline->getLayout(), myUniformManger->getDescriptorSet(myCurrentFrame));
			//myCommandBuffers[i]->bindVertexBuffer({ myModel->getVertexBuffer()->getBuffer() });
			myCommandBuffers[i]->bindVertexBuffer(myModel->getVertexBuffers());
			myCommandBuffers[i]->bindIndexBuffer(myModel->getIndexBuffer()->getBuffer());
			myCommandBuffers[i]->drawIndex(static_cast<uint32_t>(myModel->getIndexCount()));
			myCommandBuffers[i]->endRenderPass();
			myCommandBuffers[i]->end();
		}
	}

	void Application::createSemaphore() {
		for (int i = 0; i < mySwapChain->getImageCount(); ++i) {
			auto imagesemaphore = Wrapper::semaphore::create(myDevice);
			myImageAvaliablesemaphores.push_back(imagesemaphore);

			auto rendersemaphore = Wrapper::semaphore::create(myDevice);
			myRenderFinishedsemaphores.push_back(rendersemaphore);

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

		mySwapChain = Wrapper::SwapChain::create(myDevice, myWindow, mySurface);
		myWidth = mySwapChain->getExtent().width;
		myHeight = mySwapChain->getExtent().height;

		myRenderPass = Wrapper::RenderPass::create(myDevice);
		createRenderPass();

		mySwapChain->createFrameBuffer(myRenderPass);

		myPipeline = Wrapper::Pipeline::create(myDevice, myRenderPass);
		createPipeline();

		myCommandBuffers.resize(mySwapChain->getImageCount());
		createCommandBuffer();

		createSemaphore();
	}

	void Application::cleanupSwapChain() {
		mySwapChain.reset();
		myCommandBuffers.clear();
		myPipeline.reset();
		myRenderPass.reset();
		myImageAvaliablesemaphores.clear();
		myRenderFinishedsemaphores.clear();
		myFences.clear();
	}
}