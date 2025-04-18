file(GLOB_RECURSE VULKAN ./ *.cpp)	#将该文件夹所有cpp文件加入VULKAN（类型为RECURSE，表示再有文件夹也要深入找下去）

add_library(vulkanLib ${VULKAN} "device.h" "device.cpp" "window.cpp" "window.h" "windowSurface.h" "swapChain.cpp" "swapChain.h")	#将VULKAN加入library