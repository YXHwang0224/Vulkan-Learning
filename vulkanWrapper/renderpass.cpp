#include "renderpass.h"

namespace FF::Wrapper {
	SubPass::SubPass() {

	}

	SubPass::~SubPass(){

	}

	void SubPass::addColorAttachmentReference(const VkAttachmentReference& ref) {
		myColorAttachmentReferences.push_back(ref);
	}

	void SubPass::addInputAttachmentReference(const VkAttachmentReference& ref) {
		myInputAttachmentReferences.push_back(ref);
	}

	void SubPass::setDepthStencilAttachmentReference(const VkAttachmentReference& ref) {
		myDepthStencilAttachmentReference = ref;
	}

	void SubPass::setResolveAttachmentReference(const VkAttachmentReference& ref) {
		myResolveAttachmentReference = ref;
	}

	void SubPass::buildSubPassDescription() {
		if (myColorAttachmentReferences.empty()) {
			throw std::runtime_error("Error: color attachment group is empty");
		}
		mySubPassDiscription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		mySubPassDiscription.colorAttachmentCount = static_cast<uint32_t>(myColorAttachmentReferences.size());
		mySubPassDiscription.pColorAttachments = myColorAttachmentReferences.data();
		mySubPassDiscription.inputAttachmentCount = static_cast<uint32_t>(myInputAttachmentReferences.size());
		mySubPassDiscription.pInputAttachments = myInputAttachmentReferences.data();
		mySubPassDiscription.pResolveAttachments = &myResolveAttachmentReference;
		mySubPassDiscription.pDepthStencilAttachment = 
			myDepthStencilAttachmentReference.layout == VK_IMAGE_LAYOUT_UNDEFINED ? nullptr : &myDepthStencilAttachmentReference;
	}

	RenderPass::RenderPass(const Device::Ptr& device) {
		mydevice = device;
	}

	RenderPass::~RenderPass() {
		if (myRenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(mydevice->getDevice(), myRenderPass, nullptr);
		}
	}

	void RenderPass::addSubPass(SubPass& subpass) {
		mySubPasses.push_back(subpass);
	}

	void RenderPass::addDependency(VkSubpassDependency& dependency) {
		mySubPassDependencies.push_back(dependency);
	}

	void RenderPass::addAttachment(VkAttachmentDescription& attachmentDes) {
		myAttachmentDescriptions.push_back(attachmentDes);
	}

	void RenderPass::buildRenderPass() {
		if (mySubPasses.empty()||myAttachmentDescriptions.empty()||mySubPassDependencies.empty()) {
			throw std::runtime_error("Error: not enough elements to build renderPass");
		}

		//½â°üSubPass
		std::vector<VkSubpassDescription> subPasses{};
		for (int i = 0; i < mySubPasses.size(); ++i) {
			subPasses.push_back(mySubPasses[i].getSubPassDiscription());
		}

		VkRenderPassCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

		createInfo.attachmentCount = static_cast<uint32_t>(myAttachmentDescriptions.size());
		createInfo.pAttachments = myAttachmentDescriptions.data();

		createInfo.dependencyCount = static_cast<uint32_t>(mySubPassDependencies.size());
		createInfo.pDependencies = mySubPassDependencies.data();

		createInfo.subpassCount = static_cast<uint32_t>(subPasses.size());
		createInfo.pSubpasses = subPasses.data();

		if (vkCreateRenderPass(mydevice->getDevice(), &createInfo, nullptr, &myRenderPass)) {
			throw std::runtime_error("Error: failed to create renderPass");
		}
	}
}