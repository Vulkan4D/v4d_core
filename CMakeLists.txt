# V4D Core
file(GLOB_RECURSE V4DCoreSourceFiles **.cpp)
if(ENABLE_IMGUI)
	add_library(v4d SHARED 
		${V4DCoreSourceFiles}
		"${PROJECT_SOURCE_DIR}/src/imgui/imgui.cpp"
		"${PROJECT_SOURCE_DIR}/src/imgui/imgui_widgets.cpp"
		"${PROJECT_SOURCE_DIR}/src/imgui/imgui_draw.cpp"
		"${PROJECT_SOURCE_DIR}/src/imgui/examples/imgui_impl_glfw.cpp"
	)
	target_compile_definitions(v4d
		PUBLIC -D_ENABLE_IMGUI
		PUBLIC -DIMGUI_USER_CONFIG="imgui_v4d_config.hh"
	)
else()
	add_library(v4d SHARED 
		${V4DCoreSourceFiles}
	)
endif()
target_link_libraries(v4d
	PRIVATE
		ssl
		crypto
	PUBLIC
		pthread
		glfw
)
target_include_directories(v4d
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
		"${PROJECT_SOURCE_DIR}/src"
		"${PROJECT_SOURCE_DIR}/src/v4d/xvk"
		"${PROJECT_SOURCE_DIR}/src/v4d/xvk/glm"
		"${PROJECT_SOURCE_DIR}/src/v4d/xvk/gli"
		"${PROJECT_SOURCE_DIR}/src/v4d/xvk/glfw/include"
		"${PROJECT_SOURCE_DIR}/src/imgui"
	PRIVATE
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
		PRIVATE
			wsock32
			ws2_32
		PUBLIC
			gcc
			-static-libstdc++
		# INTERFACE
		# 	stdc++
	)
else()
	target_link_libraries(v4d
		PUBLIC
			dl
	)
endif()

# OpenSSL
find_package(OpenSSL 1.1.0 REQUIRED)
