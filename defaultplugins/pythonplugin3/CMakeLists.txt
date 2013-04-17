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

message ( STATUS " Configuring the Python 3 plugin " )

project( pythonplugin3 )
cmake_minimum_required( VERSION 2.8.9 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DPYTHONPLUGIN3_LIBRARY -DQT_NO_KEYWORDS)

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

find_package(PythonLibs 3.2 REQUIRED) 

IF(PYTHONLIBS_FOUND)
	include_directories (
		${PYTHON_INCLUDE_DIRS}
		../../libtransform/)

	link_directories (
		${PYTHON_LIBRARY}
		${PROJECT_SOURCE_DIR}/../../lib
		)

	set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

	set ( pythonplugin3_HDRS
		pythonplugin3_global.h
		python3.h
		pythonplugin3.h
		)

	set ( pythonplugin3_SRCS
		python3.cpp
		pythonplugin3.cpp
		)

	if(WIN32)
		link_directories (
			# cmake on windows does not configure the path correctly
			C:\\Python33\\libs  
			)
		set ( EXTERN_LIB transform python33 )
	else(WIN32)
		set ( EXTERN_LIB transform python3.2 )
	endif(WIN32)

	add_library ( pythonplugin3 MODULE ${pythonplugin3_SRCS} )

	if(WITH_QT5)
	        qt5_use_modules( pythonplugin3  Widgets Svg )
	else(WITH_QT5)
		target_link_libraries ( pythonplugin3 ${QT_LIBRARIES} )
	endif(WITH_QT5)

	target_link_libraries ( pythonplugin3 ${EXTERN_LIB} )

	IF(UNIX)
		install (TARGETS pythonplugin3 DESTINATION share/pip3line/
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
	ENDIF(UNIX)
ENDIF(PYTHONLIBS_FOUND)
