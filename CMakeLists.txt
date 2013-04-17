####
# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information
####

message( STATUS " Configuring everything " )

project( pip3lineall )
cmake_minimum_required( VERSION 2.8 )
set ( CMAKE_BUILD_TYPE Release )

add_definitions(-DQT_NO_WARNING_OUTPUT -DQT_NO_DEBUG_OUTPUT)

if (MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /GS /DYNAMICBASE")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP /GS /DYNAMICBASE")
	message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
	message(STATUS "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
endif()

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories ( ${PROJECT_SOURCE_DIR}/libtransform )

link_directories (
	${PROJECT_SOURCE_DIR}/lib
	${PROJECT_SOURCE_DIR}/bin
        ${PROJECT_SOURCE_DIR}/bin/plugins
	)

add_subdirectory(libtransform)
add_subdirectory(defaultplugins/baseplugins)
add_subdirectory(gui)
add_subdirectory(cmdline)
if ( WITH_OPENSSL ) 
	add_subdirectory(defaultplugins/opensslplugin)
endif ( WITH_OPENSSL )

if ( WITH_PYTHON27 ) 
	add_subdirectory(defaultplugins/pythonplugin27)
endif ( WITH_PYTHON27 )

if ( WITH_PYTHON3 ) 
	add_subdirectory(defaultplugins/pythonplugin3)
endif ( WITH_PYTHON3 )

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
