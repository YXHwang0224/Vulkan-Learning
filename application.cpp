#include "application.h"

namespace FF {

	void Application::Run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::initWindow() {
		myWindow = Wrapper::Window::create(WIDTH, HEIGHT);
	}

	void Application::initVulkan() {
		myInstance = Wrapper::Instance::creat(true);
		mySurface = Wrapper::WindowSurface::create(myInstance, myWindow);

		myDevice = Wrapper::Device::create(myInstance, mySurface);

		mySwapChain = Wrapper::SwapChain::create(myDevice, myWindow, mySurface);

		myRenderPass = Wrapper::RenderPass::create(myDevice);

		myPipeline = Wrapper::Pipeline::create(myDevice, myRenderPass);

		createRenderPass();

		mySwapChain->createFrameBuffer(myRenderPass);

		createPipeline();
	}

	void Application::createPipeline() {
		//�����ӿ�
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)WIDTH;
		viewport.height = (float)HEIGHT;
		//����Ļ�����У�z�᷶ΧΪ0��1
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//���ü���
		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = { WIDTH,HEIGHT };

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
		myPipeline->myVertexInputState.vertexBindingDescriptionCount = 0;
		myPipeline->myVertexInputState.pVertexBindingDescriptions = nullptr;
		myPipeline->myVertexInputState.vertexAttributeDescriptionCount = 0;
		myPipeline->myVertexInputState.pVertexAttributeDescriptions = nullptr;

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
		myPipeline->myLayoutState.setLayoutCount = 0;
		myPipeline->myLayoutState.pSetLayouts = nullptr;
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

	void Application::mainLoop() {
		while (!myWindow->shouldClose()) {
			myWindow->pollEvents();		//�ַ������̵���Ϣ
		}
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
}