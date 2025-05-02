#pragma once

#include "../base.h"

namespace FF::Wrapper {

	//Instance������Application�����Ϣ��Layer�������Ϣ����create��destroy������Ҫȡ��������չ�����Ϣ

	class Instance 
	{
	public:
		using Ptr = std::shared_ptr<Instance>;

		static Ptr create(bool enableValidationLayer) {
			return std::make_shared<Instance>(enableValidationLayer);
		}

		Instance(bool enableValidationLayer);
		~Instance();

		//�����ж�������չ
		void printAvailableExtensions();

		//����������Ҫ��������չ
		std::vector<const char*> getRequiredExtensions();

		//����Ƿ�֧��validationLayer
		bool checkValidationLayerSupport();

		//����myDebugger
		void setupDebugger();

		[[nodiscard]]VkInstance getInstance() const {		//nodiscard��ʾ���øú��������Է���ֵ���в���
			return myInstance;
		}

		[[nodiscard]] bool getEnableValidationLayer() const {	//��ȡ�Ƿ�ʹ�ü���
			return myEnableValidationLayer;
		}

	private:
		VkInstance myInstance{ VK_NULL_HANDLE };
		bool myEnableValidationLayer{ false };
		VkDebugUtilsMessengerEXT myDebugger{ VK_NULL_HANDLE };
	};
}