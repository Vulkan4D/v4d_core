# V4D Core
file(GLOB_RECURSE V4DCoreSourceFiles **.cpp)
add_library(v4d SHARED 
	${V4DCoreSourceFiles}
)
target_link_libraries(v4d
	ssl
	crypto
	glfw
	# vulkan
	# glslang
	# SPIRV
)
target_include_directories(v4d
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Vulkan-Hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Vulkan-Hpp/glm"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Vulkan-Hpp/glslang"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Vulkan-Hpp/glfw/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Vulkan-Hpp/Vulkan-Headers/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/openssl/include"
)
target_compile_definitions(v4d
	PRIVATE -D_V4D_CORE
)
set_target_properties(v4d
	PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release"
)

# Windows-Only libs
if(WIN32)
	target_link_libraries(v4d
		wsock32
		ws2_32
		gcc
		-static-libstdc++
	)
endif()

# OpenSSL
find_package(OpenSSL 1.1.0 REQUIRED)

# glfw
find_package(glfw3 3.3 REQUIRED)
#set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#add_subdirectory(glfw)

# Vulkan
# find_package(Vulkan 1.1 REQUIRED)
#add_subdirectory(Vulkan-Hpp/Vulkan-Headers)
