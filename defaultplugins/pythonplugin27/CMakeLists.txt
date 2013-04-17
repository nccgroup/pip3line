####
# pip3line: easy encoder/decoder and more
# Copyright (C) 2012-2013  Gabriel Caudrelier<gabriel.caudrelier@gmail.com>
#
# Pip3line is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Pip3line is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Pip3line.  If not, see <http://www.gnu.org/licenses/>.
####

message ( STATUS " Configuring the Python 2.7 plugin " )

project( python27plugin )
cmake_minimum_required( VERSION 2.8.9 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DPYTHONPLUGIN27_LIBRARY )

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


find_package(PythonLibs 2.7 REQUIRED) 

IF(PYTHONLIBS_FOUND)
	include_directories (
		${PYTHON_INCLUDE_DIRS}
		../../libtransform/)

	link_directories (
		${PYTHON_LIBRARY}
		${PROJECT_SOURCE_DIR}/../../lib
		)

	set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

	set ( python27plugin_HDRS
		pythonplugin27_global.h
		python27.h
		pythonplugin27.h
		)

	set ( python27plugin_SRCS
		python27.cpp
		pythonplugin27.cpp
		)

	if(WIN32)
		link_directories (
			# cmake on windows does not configure the path correctly
			C:\\Python27\\libs  
			)
		set ( EXTERN_LIB transform python27 )
	else(WIN32)
		set ( EXTERN_LIB transform python2.7 )
	endif(WIN32)

	add_library ( python27plugin MODULE ${python27plugin_SRCS} )

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
ENDIF(PYTHONLIBS_FOUND)
