# V4D Core
file(GLOB_RECURSE V4DCoreSourceFiles **.cpp)
if(ENABLE_IMGUI)
	add_library(v4d SHARED 
		${V4DCoreSourceFiles}
		"${PROJECT_SOURCE_DIR}/src/imgui/backends/imgui_impl_glfw.cpp"
		"${PROJECT_SOURCE_DIR}/src/imgui/imgui.cpp"
		"${PROJECT_SOURCE_DIR}/src/imgui/imgui_widgets.cpp"
		"${PROJECT_SOURCE_DIR}/src/imgui/imgui_draw.cpp"
		"${PROJECT_SOURCE_DIR}/src/imgui/imgui_tables.cpp"
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
		"${PROJECT_SOURCE_DIR}/src"
		"${PROJECT_SOURCE_DIR}/src/v4d/xvk"
		"${PROJECT_SOURCE_DIR}/src/imgui"
	PRIVATE
		# "${PROJECT_SOURCE_DIR}/src/openssl/include"
)
target_compile_definitions(v4d
	PRIVATE -D_V4D_CORE
)
set_target_properties(v4d
	PROPERTIES
		COMPILE_FLAGS ${BUILD_FLAGS}
		LIBRARY_OUTPUT_DIRECTORY_DEBUG "${V4D_PROJECT_BUILD_DIR}/debug"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${V4D_PROJECT_BUILD_DIR}/debug"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE "${V4D_PROJECT_BUILD_DIR}/release"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${V4D_PROJECT_BUILD_DIR}/release"
)

# Platform-specific libs
if(WIN32)
	target_link_libraries(v4d
		PRIVATE
			wsock32
			ws2_32
		PUBLIC
			gcc
			stdc++
			dbghelp
	)
else()
	target_link_libraries(v4d
		PUBLIC
			dl
	)
endif()

# OpenSSL
find_package(OpenSSL 1.1.1 REQUIRED)
