####
# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information
####

cmake_policy(VERSION 2.8.10)

message( STATUS "Git information --" )

execute_process(COMMAND git branch
	OUTPUT_VARIABLE GIT_BRANCH)

STRING(REGEX REPLACE "(\r?\n)+$" "" GIT_BRANCH "${GIT_BRANCH}")
message(STATUS "Git branches: \n ${GIT_BRANCH}")


execute_process(COMMAND git rev-parse HEAD
	OUTPUT_VARIABLE GIT_COMMIT)

STRING(REGEX REPLACE "(\r?\n)+$" "" GIT_COMMIT "${GIT_COMMIT}")
if (GIT_COMMIT STREQUAL "")
	SET(GIT_COMMIT "Unknown")
endif()

add_definitions(-DGIT_COMMIT=\"${GIT_COMMIT}\")
message(STATUS "Building using git commit: ${GIT_COMMIT}")

message( STATUS "General Configuration --" )

project( pip3lineall )
cmake_minimum_required( VERSION 2.8 )
set ( CMAKE_BUILD_TYPE Release )

add_definitions(-DQT_NO_DEBUG_OUTPUT)

if (MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /GS /DYNAMICBASE")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP /GS /DYNAMICBASE")
	message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
	message(STATUS "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
endif(MSVC)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if (WIN32)
	set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
endif(WIN32)

include_directories ( ${PROJECT_SOURCE_DIR}/libtransform )

if (WIN32)
	link_directories (
		${project_source_dir}/lib
		${project_source_dir}/bin
        	${project_source_dir}/bin/plugins
		)
else()
	link_directories (
		${project_source_dir}/bin
        	${project_source_dir}/bin/plugins
		)
endif()

if (ALL)
	message ( STATUS  "Selecting all packages")
	SET (WITH_LIB ON)
	SET (WITH_GUI ON)
	SET (WITH_CMD ON)
	SET (WITH_OPENSSL ON)
	SET (WITH_PYTHON27 ON)
	SET (WITH_PYTHON3 ON)

	if (WITH_DISTORM STREQUAL "OFF")
		message(WARNING "The Distorm plugin has been manually disabled")
	else()
		SET (WITH_DISTORM ON)
	endif()

	if (WITH_EAMPLE STREQUAL "OFF")
		message(WARNING "The example plugin has been manually disabled")
	else()
		SET (WITH_EXAMPLE ON)
	endif()

elseif ( BASIC )
	message ( STATUS "Selecting only base packages ")
	SET (WITH_LIB ON)
	SET (WITH_GUI ON)
	SET (WITH_CMD ON)
endif ()

if (WITH_LIB OR WITH_GUI OR WITH_CMD)
	add_subdirectory(libtransform)
	add_subdirectory(defaultplugins/baseplugins)
endif ()

if ( WITH_GUI )
	add_subdirectory(gui)
endif ( WITH_GUI )

if ( WITH_CMD )
	add_subdirectory(cmdline)
endif( WITH_CMD )

if ( WITH_OPENSSL ) 
	add_subdirectory(defaultplugins/opensslplugin)
endif ( WITH_OPENSSL )

if ( WITH_PYTHON3 ) 
	add_subdirectory(defaultplugins/pythonplugin)
endif ( WITH_PYTHON3 )

if ( WITH_PYTHON27 ) 
	add_subdirectory(defaultplugins/python27plugin)
endif ( WITH_PYTHON27 )

if ( WITH_DISTORM ) 
	add_subdirectory(defaultplugins/distormplugin)
endif ( WITH_DISTORM )

if ( WITH_EXAMPLE ) 
	add_subdirectory(defaultplugins/exampleplugin)
endif ( WITH_EXAMPLE )


# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
