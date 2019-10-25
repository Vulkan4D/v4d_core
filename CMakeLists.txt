# V4D Core
file(GLOB_RECURSE V4DCoreSourceFiles **.cpp)
add_library(v4d SHARED 
	${V4DCoreSourceFiles}
)
target_link_libraries(v4d
	ssl
	crypto
	glfw
	# glslang
	# SPIRV
)
target_include_directories(v4d
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
	PRIVATE
		# "${PROJECT_SOURCE_DIR}/src/Vulkan-Hpp"
		# "${PROJECT_SOURCE_DIR}/src/Vulkan-Hpp/glm"
		# "${PROJECT_SOURCE_DIR}/src/Vulkan-Hpp/glslang"
		# "${PROJECT_SOURCE_DIR}/src/Vulkan-Hpp/glfw/include"
		# "${PROJECT_SOURCE_DIR}/src/Vulkan-Hpp/Vulkan-Headers/include"
		# "${PROJECT_SOURCE_DIR}/src/openssl/include"
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

# Vulkan
if(LINK_VULKAN_HEADERS)
	if(COMPILE_VULKAN_HEADERS)
		target_link_libraries(v4d
			vulkan
			Vulkan-Headers
		)
	else()
		target_link_libraries(v4d
			vulkan
		)
	endif()
endif()
