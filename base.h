#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <string>
#include <map>
#include <set>
#include <optional>
#include <fstream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN		//开启对vulkan的支持
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

const std::vector<const char*> validationLayer{			//创建验证层
		"VK_LAYER_KHRONOS_validation"
};

struct VPMatrices {
	glm::mat4 myViewMatrix;
	glm::mat4 myProjectionMatrix;

	VPMatrices() {
		myViewMatrix = glm::mat4(1.0f);
		myProjectionMatrix = glm::mat4(1.0f);
	}
};

struct ObjectUniform {
	glm::mat4 myModelMatrix;
	
	ObjectUniform() {
		myModelMatrix = glm::mat4(1.0f);
	}
};
