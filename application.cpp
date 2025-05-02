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
			myWindow->pollEvents();		//�ַ������̵���Ϣ

			myModel->update();

			myUniformManger->update(myVPMatrices, myModel->getUniform(), myCurrentFrame);

			render();
		}

		//�ȴ�device���в���������������
		vkDeviceWaitIdle(myDevice->getDevice());
	}

	void Application::render() {
		//�ȴ���ǰҪ�ύ��commandbufferִ�����
		myFences[myCurrentFrame]->block();

		//���Ȼ�ȡ����������һ֡
		uint32_t imageIndex{ 0 };
		VkResult result = vkAcquireNextImageKHR(		//����������Ϊ��ʱʱ��
			myDevice->getDevice(),
			mySwapChain->getSwapChain(),
			UINT64_MAX,
			myImageAvaliablesemaphores[myCurrentFrame]->getsemaphore(),
			VK_NULL_HANDLE,
			&imageIndex);

		//��ⴰ�巢���ߴ�任
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			myWindow->myWindowResized = false;
		}
		//VK_SUBOPTIMAL_KHR���õ���һ����Ϊ���õ�ͼ�񣬵��Ǳ����ʽ������ȫƥ��
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Error: failes to acquire next image");
		}

		//�����ύ��Ϣ
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//ͬ����Ϣ����Ⱦ������ʾͼ�����������ʾ��Ϻ���������ɫ
		VkSemaphore waitsemaphores[] = { myImageAvaliablesemaphores[myCurrentFrame]->getsemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitsemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//ָ���ύ��Щ����
		auto commandBuffer = myCommandBuffers[imageIndex]->getCommandBuffer();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		//ִ����Ϻ󼤷���Щsemaphore
		VkSemaphore signalsemaphore[] = { myRenderFinishedsemaphores[myCurrentFrame]->getsemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalsemaphore;

		myFences[myCurrentFrame]->resetFence();

		if (vkQueueSubmit(myDevice->getGraphicQueue(), 1, &submitInfo, myFences[myCurrentFrame]->getFence()) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to submit render command");
		}

		//������ʾ��Ϣ
		VkPresentInfoKHR presentinfo{};
		presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentinfo.waitSemaphoreCount = 1;
		presentinfo.pWaitSemaphores = signalsemaphore;

		//������ʾʱʹ�õĽ�������Ϣ
		VkSwapchainKHR swapChains[] = { mySwapChain->getSwapChain() };
		presentinfo.swapchainCount = 1;
		presentinfo.pSwapchains = swapChains;

		presentinfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(myDevice->getPresentQueue(), &presentinfo);

		//������������һ����ȷ���������ǻ���Ҫʹ��Window�ж���ı�־λ�����ж�
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
		//�����ӿ�
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)myWidth;
		viewport.height = (float)myHeight;
		//����Ļ�����У�z�᷶ΧΪ0��1
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//���ü���
		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = { myWidth, myHeight };

		//�����ӿںͼ�����pipeline��
		myPipeline->setViewports({ viewport });
		myPipeline->setScissors({ scissor });

		//����shader
		std::vector<Wrapper::Shader::Ptr> shaderGroup{};
		auto vertexShader = Wrapper::Shader::create(myDevice, "shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		shaderGroup.push_back(vertexShader);
		auto fragmentShader = Wrapper::Shader::create(myDevice, "shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		shaderGroup.push_back(fragmentShader);

		myPipeline->setShaderGroup(shaderGroup);

		//�����Ų�ģʽ
		auto vertexBindingDescription = myModel->getVertexInputBindingDescription();
		auto attributeDescription = myModel->getAttributeDescription();

		myPipeline->myVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescription.size());
		myPipeline->myVertexInputState.pVertexBindingDescriptions = vertexBindingDescription.data();
		myPipeline->myVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		myPipeline->myVertexInputState.pVertexAttributeDescriptions = attributeDescription.data();

		//ͼԪװ��
		myPipeline->myAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		myPipeline->myAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		//����ͼԪ���˽ṹ���γɵ�ͼԪ
		myPipeline->myAssemblyState.primitiveRestartEnable = VK_FALSE;			//������ͼԪ

		//��դ������
		myPipeline->myRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		myPipeline->myRasterState.polygonMode = VK_POLYGON_MODE_FILL;		//���͹�դ��ͼԪѡ�����ѡ����Ҫ����GPU����
		myPipeline->myRasterState.lineWidth = 1.0f;		//����1��Ҫ����GPU����
		myPipeline->myRasterState.cullMode = VK_CULL_MODE_BACK_BIT;			//�����޳�
		myPipeline->myRasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;		//����������

		myPipeline->myRasterState.depthBiasEnable = VK_FALSE;			//�ı������Ϣ
		myPipeline->myRasterState.depthBiasConstantFactor = 0.0f;
		myPipeline->myRasterState.depthBiasClamp = 0.0f;
		myPipeline->myRasterState.depthBiasSlopeFactor = 0.0f;

		//���ز���
		myPipeline->mySampleState.sampleShadingEnable = VK_FALSE;
		myPipeline->mySampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		myPipeline->mySampleState.minSampleShading = 1.0f;
		myPipeline->mySampleState.pSampleMask = nullptr;
		myPipeline->mySampleState.alphaToCoverageEnable = VK_FALSE;
		myPipeline->mySampleState.alphaToOneEnable = VK_FALSE;

		//�����ģ�����


		//��ɫ���


		//�������ɫ������룬�õ��Ļ�Ͻ��������ͨ�����������AND���������
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	//���������ʾ��Ŀ����ɫ��ĳ����ɫ��˺������ʹ�øò������ñ�ʾֱ��ʹ��ԭ������
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	//��ʾ�����
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;		//��ʾ�����������ӽ���

		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		myPipeline->pushBlendAttachment(blendAttachment);

		//blending�Ĳ��컯�ӿڣ���application�����趨
		//1.blend�����ַ�ʽ����һ���ϣ�����alphaΪ���������㣬�ڶ���Ϊ�߼�λ���㣬��ͼ�����Ľ�����Ӵ���
		//2.���������logicOp����ô�Ϸ����õ�alphaΪ����������ʧЧ
		//3.ColorWrite���룬��Ȼ��Ч����ʹ������logicOp
		//4.��Ϊ���ǿ��ܻ��ж��FrameBuffer��������Կ�����Ҫ���BlendAttachment
		myPipeline->myBlendState.logicOpEnable = VK_FALSE;
		myPipeline->myBlendState.logicOp = VK_LOGIC_OP_COPY;

		//���blendAttachment��factor��operation
		myPipeline->myBlendState.blendConstants[0] = 0.0f;
		myPipeline->myBlendState.blendConstants[1] = 0.0f;
		myPipeline->myBlendState.blendConstants[2] = 0.0f;
		myPipeline->myBlendState.blendConstants[3] = 0.0f;

		//uniform�Ĵ���
		myPipeline->myLayoutState.setLayoutCount = 1;

		auto layout = myUniformManger->getDescriptorLayout()->getLayout();
		myPipeline->myLayoutState.pSetLayouts = &layout;
		myPipeline->myLayoutState.pushConstantRangeCount = 0;
		myPipeline->myLayoutState.pPushConstantRanges = nullptr;

		myPipeline->build();
	}

	void Application::createRenderPass() {
		//���뻭��������
		VkAttachmentDescription attachmentDes{};
		attachmentDes.format = mySwapChain->getFormat();
		attachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;		//����ʱ����в������ݣ������
		attachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;	//���ʱ����в������ݣ�����
		attachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	//���ճ�RenderPassʱ�ĸ�ʽ

		myRenderPass->addAttachment(attachmentDes);

		//���ڻ������������ü���ʽҪ��
		VkAttachmentReference attachmentReference{};
		attachmentReference.attachment = 0;
		attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	//��ʾGPU�ڽ��ø��ż���SubPassʱ����תΪʲô��ʽ

		//����SubPass
		Wrapper::SubPass subPass{};
		subPass.addColorAttachmentReference(attachmentReference);
		subPass.buildSubPassDesciption();

		myRenderPass->addSubPass(subPass);

		//SubPass��������ϵ
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;	//��һ������Ϊ��������
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	//��һ��subpass�����������ʱ������һ��
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
			renderBeginInfo.renderArea.offset = { 0,0 };	//������ʼ�㣬�ӣ�0��0���㿪ʼ
			renderBeginInfo.renderArea.extent = mySwapChain->getExtent();

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f };	//ÿ֡����ǰ��ʹ�ú�����ɫ������
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