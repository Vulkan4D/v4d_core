# V4D Core
file(GLOB_RECURSE V4DCoreSourceFiles **.cpp)
add_library(v4d SHARED 
	${V4DCoreSourceFiles}
)
target_link_libraries(v4d
	ssl
	crypto
	pthread
	glfw
)
target_include_directories(v4d
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/src/v4d/xvk"
		"${PROJECT_SOURCE_DIR}/src/v4d/xvk/glm"
	PRIVATE
		"${PROJECT_SOURCE_DIR}/src/v4d/xvk/glfw/include"
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

# Platform-specific libs
if(WIN32)
	target_link_libraries(v4d
		wsock32
		ws2_32
		gcc
		-static-libstdc++
	)
else()
	target_link_libraries(v4d
		dl
	)
endif()

# OpenSSL
find_package(OpenSSL 1.1.0 REQUIRED)
