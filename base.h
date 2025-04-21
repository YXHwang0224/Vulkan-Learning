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

#include <glm/glm.hpp>

#define GLFW_INCLUDE_VULKAN		//开启对vulkan的支持
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

const std::vector<const char*> validationLayer{			//创建验证层
		"VK_LAYER_KHRONOS_validation"
};
