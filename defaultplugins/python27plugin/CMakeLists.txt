####
# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information
####

message ( STATUS "Configuring the Python 2.7 plugin --" )
project( python27plugin )

cmake_minimum_required( VERSION 2.8.11 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DPYTHONPLUGIN_LIBRARY -DQT_NO_KEYWORDS)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WITH_QT5)
        find_package(Qt5Widgets)
        find_package(Qt5Core)
        find_package(Qt5Svg)
else(WITH_QT5)
        find_package ( Qt4 REQUIRED QtCore QtGui QtSvg)
	include ( ${QT_USE_FILE} )
endif(WITH_QT5)

if(WIN32)
	set (PYTHON_INCLUDE_DIRS C:\\Python27\\include)
	set (PYTHON_LIBRARY C:\\Python27\\libs )
endif(WIN32)

include_directories (
	${PYTHON_INCLUDE_DIRS}
	../../libtransform/)

link_directories (
	${PYTHON_LIBRARY}
	${PROJECT_SOURCE_DIR}/../../lib
	)

set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

set ( pythonplugin_HDRS
	../pythonplugin/pythonplugin_global.h
	../pythonplugin/pythontransform.h
	../pythonplugin/pythonplugin.h
	)

set ( pythonplugin_SRCS
	../pythonplugin/pythontransform.cpp
	../pythonplugin/pythonplugin.cpp
	)

if(WIN32)
	set ( EXTERN_LIB transform python27 )
else(WIN32)
	set ( EXTERN_LIB transform python2.7 )
endif(WIN32)

add_library ( python27plugin MODULE ${pythonplugin_SRCS} )

if(WITH_QT5)
	qt5_use_modules( python27plugin  Widgets Svg )
else(WITH_QT5)
	target_link_libraries ( python27plugin ${QT_LIBRARIES} )
endif(WITH_QT5)

target_link_libraries ( python27plugin ${EXTERN_LIB} )


IF(UNIX)
	install (TARGETS python27plugin DESTINATION share/pip3line/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF(UNIX)
