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

#define GLFW_INCLUDE_VULKAN		//������vulkan��֧��
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

const std::vector<const char*> validationLayer{			//������֤��
		"VK_LAYER_KHRONOS_validation"
};
