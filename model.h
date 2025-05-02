#pragma once

#include "base.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/device.h"

namespace FF {

	struct Vertex {
		glm::vec3 myPosition;
		glm::vec3 myColor;
	};

	class Model {
	public:
		using Ptr = std::shared_ptr<Model>;
		static Ptr create(const Wrapper::Device::Ptr& device) {
			return std::make_shared<Model>(device);
		}

		Model(const Wrapper::Device::Ptr& device) {
			/*myDatas = {
				{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f }},
				{{ 0.5f, 0.5f, 0.0f }, {0.0f, 1.0f, 0.0f}},
				{{ -0.5f, 0.5f, 0.0f }, {0.0f, 0.0f, 1.0f}}
			};*/

			myPositions = {
				0.0f, -0.5f, 0.0f,
				0.5f, 0.5f, 0.0f,
				-0.5f, 0.5f, 0.0f
			};

			myColors = {
				1.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 1.0f
			};

			myIndexDatas = { 0,1,2 };

			//myVertexBuffers = Wrapper::Buffer::createVertexBuffer(device, myDatas.size() * sizeof(Vertex), myDatas.data());

			myPositionBuffers = Wrapper::Buffer::createVertexBuffer(device, myPositions.size() * sizeof(float), myPositions.data());
			myColorBuffers = Wrapper::Buffer::createVertexBuffer(device, myColors.size() * sizeof(float), myColors.data());
			
			myIndexBuffers = Wrapper::Buffer::createIndexBuffer(device, myIndexDatas.size() * sizeof(unsigned int), myIndexDatas.data());

		}

		~Model(){}

		std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescription(){
			std::vector<VkVertexInputBindingDescription> bindingDescription{};
			/*bindingDescription.resize(1);

			bindingDescription[0].binding = 0;
			bindingDescription[0].stride = sizeof(Vertex);
			bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;*/

			bindingDescription.resize(2);

			bindingDescription[0].binding = 0;
			bindingDescription[0].stride = sizeof(float) * 3;
			bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			bindingDescription[1].binding = 1;
			bindingDescription[1].stride = sizeof(float) * 3;
			bindingDescription[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		//Attribute相关信息
		std::vector<VkVertexInputAttributeDescription> getAttributeDescription() {
			std::vector<VkVertexInputAttributeDescription> attributeDescription{};
			attributeDescription.resize(2);

			attributeDescription[0].binding = 0;
			attributeDescription[0].location = 0;
			attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			//attributeDescription[0].offset = offsetof(Vertex, myPosition);
			attributeDescription[0].offset = 0;

			//attributeDescription[0].binding = 0;
			attributeDescription[1].binding = 1;
			attributeDescription[1].location = 1;
			attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			//attributeDescription[1].offset = offsetof(Vertex, myColor);
			attributeDescription[1].offset = 0;

			return attributeDescription;
		}

		/*[[nodiscard]] auto getVertexBuffer() const {
			return myVertexBuffers;
		}*/

		[[nodiscard]] auto getVertexBuffers() const {
			std::vector<VkBuffer> buffers{ myPositionBuffers->getBuffer(),myColorBuffers->getBuffer() };
		
			return buffers;
		}

		void setModelMatrix(const glm::mat4 matrix) {
			myUniform.myModelMatrix = matrix;
		}

		void update() {
			glm::mat4 rotateMatrix = glm::mat4(1.0f);
			rotateMatrix = glm::rotate(rotateMatrix, glm::radians(myAngle), glm::vec3(0.0f, 0.0f, 1.0f));
			myUniform.myModelMatrix = rotateMatrix;

			myAngle += 0.001f;
		}

		[[nodiscard]] auto getIndexBuffer() const {
			return myIndexBuffers;
		}

		[[nodiscard]] auto getIndexCount() const {
			return myIndexDatas.size();
		}

		[[nodiscard]] auto getUniform() const {
			return myUniform;
		}

	private:
		//std::vector<Vertex> myDatas{};
		std::vector<float> myPositions{};
		std::vector<float> myColors{};

		std::vector<unsigned int> myIndexDatas{};

		//Wrapper::Buffer::Ptr myVertexBuffers{ nullptr };

		Wrapper::Buffer::Ptr myPositionBuffers{ nullptr };
		Wrapper::Buffer::Ptr myColorBuffers{ nullptr };

		Wrapper::Buffer::Ptr myIndexBuffers{ nullptr };

		ObjectUniform myUniform;

		float myAngle = 0.0f;
	};
}