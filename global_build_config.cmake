# C++20 standard, 64-bit only
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${V4D_PROJECT_BUILD_DIR}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${V4D_PROJECT_BUILD_DIR}/debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${V4D_PROJECT_BUILD_DIR}/release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${V4D_PROJECT_BUILD_DIR}/release")
set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")
set(CMAKE_SHARED_MODULE_PREFIX_CXX "")
set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "" FORCE)
set(CMAKE_MINSIZEREL_POSTFIX "" CACHE STRING "" FORCE)
set(CMAKE_RELWITHDEBINFO_POSTFIX "" CACHE STRING "" FORCE)

# Windows / Linux
if(WIN32)
	add_definitions(-D_WINDOWS -D_WIN32_WINNT=0x0A000000)
else()
	add_definitions(-D_LINUX)
endif()

# Compiler optimizations and CPU Extensions
set(BUILD_FLAGS "-mavx2")

# Compiler Warnings
if(WIN32)
	set(BUILD_FLAGS "${BUILD_FLAGS} -W")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-attribute-warning")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-cast-function-type")
else()
	# Warnings		https://gcc.gnu.org/onlinedocs/gcc-9.2.0/gcc/Warning-Options.html
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wall")
	set(BUILD_FLAGS "${BUILD_FLAGS} -fmax-errors=1")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wfatal-errors")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wextra")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wunused")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wnon-virtual-dtor")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wcast-align")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Woverloaded-virtual")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wnull-dereference")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wpessimizing-move")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wredundant-move")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wlogical-op")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wdisabled-optimization")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wfloat-conversion")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-unused-parameter")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-sign-compare")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-unknown-pragmas")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-unused-function")
	# set(BUILD_FLAGS "${BUILD_FLAGS} -Wconversion")
	# set(BUILD_FLAGS "${BUILD_FLAGS} -Wsign-conversion")
	# set(BUILD_FLAGS "${BUILD_FLAGS} -Wshadow")
endif()

# don't open a console window on Windows
if(WIN32)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()

# Remove compiler warnings caused by COMMON_OBJECT
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-invalid-offsetof")

# Remove compiler Warnings from VMA...
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-missing-field-initializers")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-type-limits")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-unused-variable")
	
# Remove compiler Warnings from tinygltfloader...
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-ignored-qualifiers")
	set(BUILD_FLAGS "${BUILD_FLAGS} -Wno-switch")

# Release / Debug
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(DEBUG 1)
	set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
	add_definitions(-D_DEBUG)
else()
	set(RELEASE 1)
	set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
	add_definitions(-D_RELEASE)
endif()


############################################
# Modules functions

function(ModuleVendor ven)
	set(_ModuleVendor "${ven}" PARENT_SCOPE)
endfunction()

function(ModuleName mod)
	set(_ModuleName "${mod}" PARENT_SCOPE)
endfunction()

function(SubModule _ModuleClass)
	if("${_ModuleDir}" STREQUAL "")
		set(_ModuleDir "${CMAKE_CURRENT_SOURCE_DIR}")
	endif()
	set(_SubModuleName "${_ModuleVendor}_${_ModuleName}.${_ModuleClass}" PARENT_SCOPE)
	set(_SubModuleName "${_ModuleVendor}_${_ModuleName}.${_ModuleClass}")
	foreach(_SourceFile ${ARGN})
		list(APPEND _SourceFiles "${_ModuleDir}/${_SourceFile}")
	endforeach()
	add_library(${_SubModuleName} MODULE ${_SourceFiles})
	if(MODULES_LINK_WITH_V4D_CORE)
		target_link_libraries(${_SubModuleName} v4d)
	endif()
	if(MODULES_LINK_WITH_GAME)
		target_link_libraries(${_SubModuleName} game)
	endif()
	target_include_directories(${_SubModuleName} PRIVATE "${_ModuleDir}")
	target_compile_definitions(${_SubModuleName}
		PRIVATE -DTHIS_MODULE="${_ModuleVendor}_${_ModuleName}"
		PRIVATE -D_V4D_MODULE
	)
	set_target_properties(${_SubModuleName}
		PROPERTIES
			COMPILE_FLAGS ${BUILD_FLAGS}
			LIBRARY_OUTPUT_DIRECTORY_DEBUG "${V4D_PROJECT_BUILD_DIR}/debug/modules/${_ModuleVendor}_${_ModuleName}"
			RUNTIME_OUTPUT_DIRECTORY_DEBUG "${V4D_PROJECT_BUILD_DIR}/debug/modules/${_ModuleVendor}_${_ModuleName}"
			LIBRARY_OUTPUT_DIRECTORY_RELEASE "${V4D_PROJECT_BUILD_DIR}/release/modules/${_ModuleVendor}_${_ModuleName}"
			RUNTIME_OUTPUT_DIRECTORY_RELEASE "${V4D_PROJECT_BUILD_DIR}/release/modules/${_ModuleVendor}_${_ModuleName}"
	)
endfunction()
