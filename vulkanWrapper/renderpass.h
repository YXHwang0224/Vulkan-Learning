#pragma once

#include "..\base.h"
#include "device.h"

namespace FF::Wrapper {

	/*
	1.Attachment:
	VkAttachmentDescription �����������ŵĽṹ����������������������Ҫʲô
	VkAttachmentReference ˵������SubPass��Ҫ������һ�����ŵ�id��ͼƬ������ʽ
	VkSubPass������дһ��SubPass�������ṹ
	VkSubPassDependency ������ͬSubPass֮���������ϵ
	*/

	class SubPass {
	public:
		SubPass();

		~SubPass();

		void addColorAttachmentReference(const VkAttachmentReference& ref);

		void addInputAttachmentReference(const VkAttachmentReference& ref);

		void setDepthStencilAttachmentReference(const VkAttachmentReference& ref);
		
		void buildSubPassDesciption();

		[[nodiscard]] auto getSubPassDiscription() const {
			return mySubPassDiscription;
		}

	private:
		VkSubpassDescription mySubPassDiscription{};
		std::vector<VkAttachmentReference> myColorAttachmentReferences{};
		std::vector<VkAttachmentReference> myInputAttachmentReferences{};
		VkAttachmentReference myDepthStencilAttachmentReference{};
	};

	class RenderPass {
	public:
		using Ptr = std::shared_ptr<RenderPass>;
		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<RenderPass>(device);
		}

		RenderPass(const Device::Ptr &device);

		~RenderPass();

		void addSubPass(SubPass& subpass);

		void addDependency(VkSubpassDependency& dependency);

		void addAttachment(VkAttachmentDescription& attachmentDes);

		void buildRenderPass();

		[[nodiscard]] auto getRenderPass() const {
			return myRenderPass;
		}

	private:
		VkRenderPass myRenderPass{ VK_NULL_HANDLE };

		std::vector<SubPass> mySubPasses{};
		std::vector<VkSubpassDependency> mySubPassDependencies{};
		std::vector<VkAttachmentDescription> myAttachmentDescriptions{};

		Device::Ptr mydevice{ nullptr };
	};
}