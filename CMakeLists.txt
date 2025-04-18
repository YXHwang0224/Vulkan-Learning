cmake_minimum_required(VERSION 3.12)						#描述版本号
project(vulkanLearning-1)									#给生成工程命名

set(CMAKE_CXX_STANDARD 17)									#规定C++版本为C++17

include_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/glm/include
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/glfw/include
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/vulkan/1.2.182.0/Include
)															#引入第三方库头文件目录

link_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/glfw/lib
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/vulkan/1.2.182.0/Lib
)															#引入静态库lib

aux_source_directory(. DIRSRCS)								#扫描该文件夹下所有文件加入DIRSRCS

add_subdirectory(vulkanWrapper)								#检索vulkanWrapper这一文件夹

add_executable(vulkanLearning-1  ${DIRSRCS} "application.h" "application.cpp" "vulkanWrapper/instance.h" "vulkanWrapper/instance.cpp")				#使用DIRSRCS创建可执行程序

target_link_libraries(vulkanLearning-1 vulkan-1.lib glfw3.lib vulkanLib)						#将引入库加入可执行文件
