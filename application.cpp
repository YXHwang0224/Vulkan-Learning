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
		//设置视口
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)WIDTH;
		viewport.height = (float)HEIGHT;
		//在屏幕坐标中，z轴范围为0到1
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//设置剪裁
		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = { WIDTH,HEIGHT };

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
		myPipeline->myVertexInputState.vertexBindingDescriptionCount = 0;
		myPipeline->myVertexInputState.pVertexBindingDescriptions = nullptr;
		myPipeline->myVertexInputState.vertexAttributeDescriptionCount = 0;
		myPipeline->myVertexInputState.pVertexAttributeDescriptions = nullptr;

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
		myPipeline->myLayoutState.setLayoutCount = 0;
		myPipeline->myLayoutState.pSetLayouts = nullptr;
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

	void Application::mainLoop() {
		while (!myWindow->shouldClose()) {
			myWindow->pollEvents();		//分发鼠标键盘等信息
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